#include "export_parser.h"
#include "generator.h"
#include "import_parser.h"
#include "pe_reader.h"
#include "report.h"
#include <stdio.h>
#include <string.h>

static void usage(FILE*f){fputs("Usage: dllproxygen --exe target.exe --dll original.dll --out DIR [options]\nOptions:\n  --scope imported|all   Export selection (default: imported)\n  --format text|json|both (default: both)\n  --strict               Fail if an import cannot be matched\n  --help\n",f);}
static const char*basename_(const char*p){const char*b=p;for(;*p;p++)if(*p=='/'||*p=='\\')b=p+1;return b;}
int main(int argc,char**argv){const char*exe=NULL,*dll=NULL,*out=NULL,*fmt="both";bool all=false,strict=false;
    for(int i=1;i<argc;i++){if(!strcmp(argv[i],"--help")){usage(stdout);return 0;}else if((!strcmp(argv[i],"--exe")||!strcmp(argv[i],"--dll")||!strcmp(argv[i],"--out")||!strcmp(argv[i],"--scope")||!strcmp(argv[i],"--format"))&&i+1<argc){const char*k=argv[i],*v=argv[++i];if(!strcmp(k,"--exe"))exe=v;else if(!strcmp(k,"--dll"))dll=v;else if(!strcmp(k,"--out"))out=v;else if(!strcmp(k,"--scope")){if(!strcmp(v,"all"))all=true;else if(strcmp(v,"imported")){fprintf(stderr,"invalid --scope: %s\n",v);return 2;}}else fmt=v;}else if(!strcmp(argv[i],"--strict"))strict=true;else if(!strcmp(argv[i],"--include-delay-imports")||!strcmp(argv[i],"--hooks")){fprintf(stderr,"option %s is not implemented in this milestone\n",argv[i]);return 2;}else{fprintf(stderr,"unknown or incomplete option: %s\n",argv[i]);usage(stderr);return 2;}}
    if(!exe||!dll||!out){usage(stderr);return 2;}bool wt=!strcmp(fmt,"text")||!strcmp(fmt,"both"),wj=!strcmp(fmt,"json")||!strcmp(fmt,"both");if(!wt&&!wj){fprintf(stderr,"invalid --format: %s\n",fmt);return 2;}
    char err[512]={0},internal[DPG_MAX_NAME];PeImage ep={0},dp={0};DpgImports im={0};DpgExports ex={0};int rc=1;
    if(!pe_open(&ep,exe,err,sizeof err)){fprintf(stderr,"EXE: %s\n",err);goto done;}if(!pe_open(&dp,dll,err,sizeof err)){fprintf(stderr,"DLL: %s\n",err);goto done;}
    if(!(ep.characteristics&0x0002U)){fprintf(stderr,"EXE input is not marked executable\n");goto done;}if(!(dp.characteristics&0x2000U)){fprintf(stderr,"DLL input is not marked as a DLL\n");goto done;}
    if(!parse_imports(&ep,basename_(dll),&im,err,sizeof err)){fprintf(stderr,"imports: %s\n",err);goto done;}if(!parse_exports(&dp,&ex,internal,sizeof internal,err,sizeof err)){fprintf(stderr,"exports: %s\n",err);goto done;}match_imports(&im,&ex);
    size_t unresolved=0;for(size_t i=0;i<im.count;i++)if(!im.items[i].matched)unresolved++;if(unresolved)fprintf(stderr,"warning: %zu import(s) could not be matched to an export\n",unresolved);if(strict&&unresolved){fprintf(stderr,"strict mode: refusing incomplete proxy\n");goto done;}
    if(!generate_proxy(out,dll,&im,&ex,all,err,sizeof err)){fprintf(stderr,"generation: %s\n",err);goto done;}if(!write_reports(out,exe,dll,&im,&ex,wt,wj,err,sizeof err)){fprintf(stderr,"report: %s\n",err);goto done;}
    printf("Generated proxy sources and analysis in %s (%zu imports, %zu unresolved).\n",out,im.count,unresolved);rc=0;
done:dpg_imports_free(&im);dpg_exports_free(&ex);pe_close(&ep);pe_close(&dp);return rc;}
