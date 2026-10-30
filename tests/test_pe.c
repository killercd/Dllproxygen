#include "export_parser.h"
#include "import_parser.h"
#include "pe_reader.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void w16(unsigned char*b,size_t o,unsigned v){b[o]=(unsigned char)v;b[o+1]=(unsigned char)(v>>8);}
static void w32(unsigned char*b,size_t o,uint32_t v){for(unsigned i=0;i<4;i++)b[o+i]=(unsigned char)(v>>(i*8));}
static void w64(unsigned char*b,size_t o,uint64_t v){w32(b,o,(uint32_t)v);w32(b,o+4,(uint32_t)(v>>32));}
static size_t ro(uint32_t rva){return 0x200U+(size_t)(rva-0x1000U);}
static unsigned char *fixture(size_t *size){
    *size=0xa00;unsigned char*b=calloc(1,*size);assert(b);b[0]='M';b[1]='Z';w32(b,0x3c,0x80);w32(b,0x80,0x4550);w16(b,0x84,0x8664);w16(b,0x86,1);w16(b,0x94,0xf0);w16(b,0x96,0x2022);size_t o=0x98;w16(b,o,0x20b);w32(b,o+56,0x2000);w32(b,o+60,0x200);w32(b,o+108,16);
    w32(b,o+112,0x1200);w32(b,o+116,0x100);w32(b,o+120,0x1000);w32(b,o+124,0x40);
    size_t s=o+0xf0;memcpy(b+s,".rdata",6);w32(b,s+8,0x800);w32(b,s+12,0x1000);w32(b,s+16,0x800);w32(b,s+20,0x200);w32(b,s+36,0x40000040);
    size_t d=ro(0x1000);w32(b,d,0x1120);w32(b,d+12,0x1100);w32(b,d+16,0x1160);memcpy(b+ro(0x1100),"sample.dll",11);w64(b,ro(0x1120),0x1140);w64(b,ro(0x1128),UINT64_C(0x8000000000000009));w16(b,ro(0x1140),3);memcpy(b+ro(0x1142),"Named",6);
    size_t e=ro(0x1200);w32(b,e+12,0x1270);w32(b,e+16,7);w32(b,e+20,3);w32(b,e+24,2);w32(b,e+28,0x1240);w32(b,e+32,0x1250);w32(b,e+36,0x1260);w32(b,ro(0x1240),0x1400);w32(b,ro(0x1244),0);w32(b,ro(0x1248),0x1280);w32(b,ro(0x1250),0x1290);w32(b,ro(0x1254),0x1298);w16(b,ro(0x1260),0);w16(b,ro(0x1262),0);memcpy(b+ro(0x1270),"sample.dll",11);memcpy(b+ro(0x1280),"other.Real",11);memcpy(b+ro(0x1290),"Named",6);memcpy(b+ro(0x1298),"Alias",6);return b;
}
int main(void){char err[256],name[64];size_t n;unsigned char*b=fixture(&n);PeImage p;assert(pe_parse_memory(&p,b,n,"fixture",err,sizeof err));size_t off;assert(pe_rva_to_offset(&p,0x1140,8,&off)&&off==ro(0x1140));assert(!pe_rva_to_offset(&p,0x1800,1,&off));DpgImports im={0};DpgExports ex={0};assert(parse_imports(&p,"SAMPLE.DLL",&im,err,sizeof err));assert(im.count==2&&!im.items[0].by_ordinal&&im.items[1].by_ordinal&&im.items[1].ordinal==9);assert(parse_exports(&p,&ex,name,sizeof name,err,sizeof err));assert(ex.count==3&&ex.items[0].has_name&&ex.items[1].is_forwarder&&ex.items[2].has_name);match_imports(&im,&ex);assert(im.items[0].matched&&im.items[1].matched);dpg_imports_free(&im);dpg_exports_free(&ex);
    assert(!pe_parse_memory(&p,b,100,"truncated",err,sizeof err));b[0]='X';assert(!pe_parse_memory(&p,b,n,"bad",err,sizeof err));b[0]='M';w16(b,0x84,0x14c);assert(!pe_parse_memory(&p,b,n,"x86",err,sizeof err));free(b);puts("test_pe: all checks passed");return 0;}
