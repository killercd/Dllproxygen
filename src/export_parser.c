#include "export_parser.h"
#include <stdio.h>
#include <string.h>

static void seterr(char*e,size_t n,const char*m){if(n)snprintf(e,n,"%s",m);}
static DpgExport *find_ord(DpgExports*v,uint16_t ord){for(size_t i=0;i<v->count;i++)if(v->items[i].ordinal==ord)return &v->items[i];return NULL;}

bool parse_exports(const PeImage*p,DpgExports*out,char*dllname,size_t namecap,char*err,size_t cap){
    uint32_t dr=p->directory_rva[0],ds=p->directory_size[0];size_t o;
    if(!dr||ds<40U||!pe_rva_to_offset(p,dr,40,&o)){seterr(err,cap,"DLL has no valid export directory");return false;}
    uint32_t name_rva,base,nfunc,nname,funcs,names,ords;
    if(!pe_read_u32(p,o+12,&name_rva)||!pe_read_u32(p,o+16,&base)||!pe_read_u32(p,o+20,&nfunc)||!pe_read_u32(p,o+24,&nname)||!pe_read_u32(p,o+28,&funcs)||!pe_read_u32(p,o+32,&names)||!pe_read_u32(p,o+36,&ords)){seterr(err,cap,"truncated export directory");return false;}
    if(base>65535U||nfunc>65536U||nname>1048576U||nname>nfunc*64U+1024U){seterr(err,cap,"unreasonable export table counts");return false;}
    if(!pe_read_cstring_rva(p,name_rva,dllname,namecap)){seterr(err,cap,"invalid export DLL name");return false;}
    size_t fo;if(nfunc&&!pe_rva_to_offset(p,funcs,(size_t)nfunc*4U,&fo)){seterr(err,cap,"invalid export address table");return false;}
    for(uint32_t i=0;i<nfunc;i++){uint32_t fr;if(!pe_read_u32(p,fo+(size_t)i*4U,&fr))return false;if(!fr)continue;if(base+i>65535U){seterr(err,cap,"export ordinal exceeds 16-bit range");return false;}DpgExport x={0};x.ordinal=(uint16_t)(base+i);x.rva=fr;x.noname=true;
        if(fr>=dr&&(uint64_t)fr<(uint64_t)dr+ds){x.is_forwarder=true;if(!pe_read_cstring_rva(p,fr,x.forwarder,sizeof x.forwarder)){seterr(err,cap,"invalid export forwarder string");return false;}}
        if(!dpg_exports_push(out,&x)){seterr(err,cap,"out of memory");return false;}}
    size_t no,oo;if(nname&&(!pe_rva_to_offset(p,names,(size_t)nname*4U,&no)||!pe_rva_to_offset(p,ords,(size_t)nname*2U,&oo))){seterr(err,cap,"invalid export name tables");return false;}
    for(uint32_t i=0;i<nname;i++){uint32_t nr;uint16_t idx;if(!pe_read_u32(p,no+(size_t)i*4U,&nr)||!pe_read_u16(p,oo+(size_t)i*2U,&idx)||idx>=nfunc){seterr(err,cap,"invalid export name ordinal index");return false;}uint32_t ord32=base+idx;if(ord32>65535U){seterr(err,cap,"named export ordinal overflow");return false;}char nm[DPG_MAX_NAME];if(!pe_read_cstring_rva(p,nr,nm,sizeof nm)){seterr(err,cap,"invalid export name");return false;}DpgExport *x=find_ord(out,(uint16_t)ord32);if(!x){seterr(err,cap,"export name refers to an address-table hole");return false;}
        if(!x->has_name){x->has_name=true;x->noname=false;snprintf(x->name,sizeof x->name,"%s",nm);}else{DpgExport alias=*x;snprintf(alias.name,sizeof alias.name,"%s",nm);alias.noname=false;if(!dpg_exports_push(out,&alias)){seterr(err,cap,"out of memory");return false;}}}
    return true;
}
void match_imports(DpgImports*im,const DpgExports*ex){for(size_t i=0;i<im->count;i++){for(size_t j=0;j<ex->count;j++){bool ok=im->items[i].by_ordinal?im->items[i].ordinal==ex->items[j].ordinal:(ex->items[j].has_name&&strcmp(im->items[i].name,ex->items[j].name)==0);if(ok){im->items[i].matched=true;im->items[i].export_index=j;break;}}}}
