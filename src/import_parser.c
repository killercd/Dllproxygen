#include "import_parser.h"
#include <ctype.h>
#include <stdio.h>
#include <string.h>

static int ci_equal(const char *a,const char*b){while(*a&&*b){if(tolower((unsigned char)*a)!=tolower((unsigned char)*b))return 0;a++;b++;}return *a==*b;}
static void seterr(char*e,size_t n,const char*m){if(n)snprintf(e,n,"%s",m);}

bool parse_imports(const PeImage *p,const char *wanted,DpgImports*out,char*err,size_t cap){
    uint32_t rva=p->directory_rva[1], bytes=p->directory_size[1];
    if(!rva||!bytes){seterr(err,cap,"executable has no static import directory");return false;}
    size_t base;if(!pe_rva_to_offset(p,rva,20,&base)){seterr(err,cap,"invalid import directory RVA");return false;}
    size_t max=bytes/20U;if(max==0||max>65536U)max=65536U;bool found=false,terminated=false;
    for(size_t di=0;di<max;di++){
        size_t o=base+di*20U;uint32_t oft,ts,fc,name_rva,ft;
        if(!pe_read_u32(p,o,&oft)||!pe_read_u32(p,o+4,&ts)||!pe_read_u32(p,o+8,&fc)||!pe_read_u32(p,o+12,&name_rva)||!pe_read_u32(p,o+16,&ft)){seterr(err,cap,"truncated import descriptor");return false;}
        if(!(oft|ts|fc|name_rva|ft)){terminated=true;break;}
        char dll[DPG_MAX_NAME];if(!pe_read_cstring_rva(p,name_rva,dll,sizeof dll)){seterr(err,cap,"invalid import DLL name");return false;}
        if(!ci_equal(dll,wanted))continue;
        found=true;
        uint32_t lookup=oft?oft:ft;if(!lookup||!ft){seterr(err,cap,"selected import descriptor has no usable thunk table");return false;}
        for(size_t ti=0;ti<1048576U;ti++){
            uint64_t thunk;size_t to;if((uint64_t)lookup+ti*8U>UINT32_MAX||!pe_rva_to_offset(p,lookup+(uint32_t)(ti*8U),8,&to)||!pe_read_u64(p,to,&thunk)){seterr(err,cap,"invalid or unterminated import thunk table");return false;}
            if(!thunk)break;
            DpgImport x={0};snprintf(x.dll_name,sizeof x.dll_name,"%s",dll);x.ilt_rva=lookup+(uint32_t)(ti*8U);x.iat_rva=ft+(uint32_t)(ti*8U);
            if(thunk&UINT64_C(0x8000000000000000)){x.by_ordinal=true;x.ordinal=(uint16_t)(thunk&0xffffU);}
            else {if(thunk>UINT32_MAX){seterr(err,cap,"bound IAT cannot be used as an import lookup table");return false;}size_t hn;if(!pe_rva_to_offset(p,(uint32_t)thunk,3,&hn)||!pe_read_u16(p,hn,&x.hint)||!pe_read_cstring_rva(p,(uint32_t)thunk+2U,x.name,sizeof x.name)){seterr(err,cap,"invalid import-by-name record");return false;}}
            if(!dpg_imports_push(out,&x)){seterr(err,cap,"out of memory");return false;}
        }
    }
    if(!terminated){seterr(err,cap,"unterminated import descriptor table");return false;}
    if(!found){seterr(err,cap,"the executable does not statically import the selected DLL basename");return false;}
    return true;
}
