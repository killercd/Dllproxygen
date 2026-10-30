#include "pe_reader.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void fail(char *err, size_t cap, const char *msg) { if (cap) snprintf(err, cap, "%s", msg); }
static bool range(size_t size, size_t off, size_t n) { return off <= size && n <= size - off; }
bool pe_read_u16(const PeImage *p, size_t o, uint16_t *v) {
    if (!range(p->size,o,2)) return false;
    *v=(uint16_t)(p->data[o]|((uint16_t)p->data[o+1]<<8));
    return true;
}
bool pe_read_u32(const PeImage *p, size_t o, uint32_t *v) {
    if (!range(p->size,o,4)) return false;
    *v=(uint32_t)p->data[o]|((uint32_t)p->data[o+1]<<8)|((uint32_t)p->data[o+2]<<16)|((uint32_t)p->data[o+3]<<24);
    return true;
}
bool pe_read_u64(const PeImage *p, size_t o, uint64_t *v) {
    uint32_t lo,hi; if(!pe_read_u32(p,o,&lo)||!pe_read_u32(p,o+4,&hi))return false; *v=(uint64_t)lo|((uint64_t)hi<<32); return true;
}
bool pe_parse_memory(PeImage *p, uint8_t *data, size_t size, const char *label, char *err, size_t cap) {
    uint32_t nt, sig, dirs; uint16_t optsz, magic, sections;
    memset(p,0,sizeof *p); p->data=data; p->size=size; snprintf(p->path,sizeof p->path,"%s",label?label:"<memory>");
    if (!range(size,0,64) || data[0]!='M' || data[1]!='Z') { fail(err,cap,"not a DOS MZ image"); return false; }
    if (!pe_read_u32(p,0x3c,&nt) || !range(size,nt,24)) { fail(err,cap,"invalid e_lfanew"); return false; }
    if (!pe_read_u32(p,nt,&sig) || sig!=0x00004550U) { fail(err,cap,"missing PE signature"); return false; }
    if (!pe_read_u16(p,nt+4,&p->machine)||!pe_read_u16(p,nt+6,&sections)||!pe_read_u16(p,nt+20,&optsz)||!pe_read_u16(p,nt+22,&p->characteristics)) { fail(err,cap,"truncated COFF header"); return false; }
    if (p->machine!=0x8664U) { fail(err,cap,"unsupported machine: only AMD64 is accepted"); return false; }
    if (sections>DPG_MAX_SECTIONS) { fail(err,cap,"too many PE sections"); return false; }
    size_t opt=(size_t)nt+24U;
    if (!range(size,opt,optsz)||optsz<112U||!pe_read_u16(p,opt,&magic)||magic!=0x20bU) { fail(err,cap,"unsupported or truncated optional header (PE32+ required)"); return false; }
    if (!pe_read_u32(p,opt+16,&p->entry_rva)||!pe_read_u32(p,opt+56,&p->image_size)||!pe_read_u32(p,opt+60,&p->size_of_headers)||!pe_read_u32(p,opt+108,&dirs)) { fail(err,cap,"truncated PE32+ optional header"); return false; }
    if (dirs>DPG_MAX_DIRECTORIES) dirs=DPG_MAX_DIRECTORIES;
    if ((size_t)optsz < 112U+(size_t)dirs*8U) { fail(err,cap,"truncated data directories"); return false; }
    for(uint32_t i=0;i<dirs;i++){ if(!pe_read_u32(p,opt+112U+i*8U,&p->directory_rva[i])||!pe_read_u32(p,opt+116U+i*8U,&p->directory_size[i])) return false; }
    size_t sh=opt+optsz;
    if (!range(size,sh,(size_t)sections*40U)) { fail(err,cap,"truncated section table"); return false; }
    p->section_count=sections;
    for(size_t i=0;i<sections;i++){
        size_t o=sh+i*40U; PeSection *s=&p->sections[i]; memcpy(s->name,data+o,8); s->name[8]=0;
        if(!pe_read_u32(p,o+8,&s->virtual_size)||!pe_read_u32(p,o+12,&s->virtual_address)||!pe_read_u32(p,o+16,&s->raw_size)||!pe_read_u32(p,o+20,&s->raw_offset)||!pe_read_u32(p,o+36,&s->characteristics)){fail(err,cap,"truncated section");return false;}
        if (s->raw_size && !range(size,s->raw_offset,s->raw_size)) { fail(err,cap,"section raw data exceeds file"); return false; }
    }
    return true;
}
bool pe_open(PeImage *p,const char *path,char *err,size_t cap){
    FILE *f=fopen(path,"rb"); if(!f){if(cap)snprintf(err,cap,"cannot open %s: %s",path,strerror(errno));return false;}
    if(fseek(f,0,SEEK_END)!=0){fclose(f);fail(err,cap,"cannot seek input");return false;} long n=ftell(f);
    if(n<0||fseek(f,0,SEEK_SET)!=0){fclose(f);fail(err,cap,"cannot size input");return false;}
    uint8_t *d=malloc((size_t)n? (size_t)n:1U); if(!d){fclose(f);fail(err,cap,"out of memory");return false;}
    if((size_t)n!=fread(d,1,(size_t)n,f)){free(d);fclose(f);fail(err,cap,"short read");return false;} fclose(f);
    if(!pe_parse_memory(p,d,(size_t)n,path,err,cap)){free(d);p->data=NULL;return false;} return true;
}
void pe_close(PeImage *p){free(p->data);memset(p,0,sizeof *p);}
bool pe_rva_to_offset(const PeImage *p,uint32_t rva,size_t need,size_t *off){
    if(rva<p->size_of_headers){if(range(p->size,rva,need)){*off=rva;return true;}return false;}
    for(size_t i=0;i<p->section_count;i++){const PeSection*s=&p->sections[i];uint32_t span=s->virtual_size>s->raw_size?s->virtual_size:s->raw_size;
        if(rva>=s->virtual_address&&(uint64_t)rva<(uint64_t)s->virtual_address+span){uint64_t delta=(uint64_t)rva-s->virtual_address;
            if(delta>s->raw_size||need>(size_t)(s->raw_size-delta))return false;
            uint64_t o=(uint64_t)s->raw_offset+delta;
            if(o>SIZE_MAX||!range(p->size,(size_t)o,need))return false;
            *off=(size_t)o;return true;}}
    return false;
}
bool pe_read_cstring_rva(const PeImage*p,uint32_t rva,char*out,size_t cap){size_t o;if(!cap||!pe_rva_to_offset(p,rva,1,&o))return false;size_t i=0;while(o+i<p->size&&i+1<cap){out[i]=(char)p->data[o+i];if(!out[i])return true;i++;}out[cap-1]=0;return false;}
