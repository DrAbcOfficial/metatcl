#include <metahook.h>
#include <com_model.h>
#include <r_studioint.h>
#include <string>
#include <map>
#include <filesystem>
#include <tcl.h>

#pragma comment(lib,"./tclib/lib/tcl86t.lib")

cl_exportfuncs_t gExportfuncs = { 0 };
mh_interface_t* g_pInterface = nullptr;
metahook_api_t* g_pMetaHookAPI = nullptr;
IFileSystem* g_pFileSystem = nullptr;
IFileSystem_HL25* g_pFileSystem_HL25 = nullptr;
cl_enginefunc_t gEngfuncs;
engine_studio_api_t IEngineStudio;

void IPluginsV4::Init(metahook_api_t *pAPI, mh_interface_t *pInterface, mh_enginesave_t *pSave){
	g_pInterface = pInterface;
	g_pMetaHookAPI = pAPI;
}
void IPluginsV4::Shutdown(void){
}
void IPluginsV4::LoadEngine(cl_enginefunc_t *pEngfuncs){
	g_pFileSystem = g_pInterface->FileSystem;
	g_pFileSystem_HL25 = g_pInterface->FileSystem_HL25;
	memcpy(&gEngfuncs, pEngfuncs, sizeof(gEngfuncs));
}

static Tcl_Interp* s_pTclinterp = nullptr;
static char s_szLibpath[MAX_PATH] = { 0 };
static char s_szPkgpath[MAX_PATH] = { 0 };
static void ResetTCLinter() {
	if(s_pTclinterp)
		Tcl_DeleteInterp(s_pTclinterp);
	s_pTclinterp = Tcl_CreateInterp();
	Tcl_SetVar(s_pTclinterp, "tcl_library", s_szLibpath, TCL_GLOBAL_ONLY);
	Tcl_SetVar(s_pTclinterp, "tcl_pkgPath", s_szPkgpath, TCL_GLOBAL_ONLY);
	Tcl_SetVar(s_pTclinterp, "tcl_interactive", "0", TCL_GLOBAL_ONLY);
	if (Tcl_Init(s_pTclinterp) == TCL_ERROR) {
		g_pMetaHookAPI->SysError("[TCL] Tcl init failed!\n%s", Tcl_GetStringResult(s_pTclinterp));
		return;
	}
	char path[MAX_PATH];
	snprintf(path, MAX_PATH, "{%s}\n{%s}", s_szLibpath, s_szPkgpath);
	Tcl_SetVar(s_pTclinterp, "auto_path", path, TCL_GLOBAL_ONLY);
	if (Tcl_SetSystemEncoding(s_pTclinterp, "utf-8") == TCL_ERROR) {
		g_pMetaHookAPI->SysError("[TCL] Tcl set utf-8 encoding failed!\n%s", Tcl_GetStringResult(s_pTclinterp));
		return;
	}
	static auto buildargstr = [](std::string& str, int argc, const char* argv[]) {
		if (argc <= 1)
			return;
		for (int i = 1; i < argc; i++) {
			str += argv[i];
			if (i < argc - 1) {
				str += " ";
			}
		}
	};
	Tcl_CreateCommand(s_pTclinterp, "puts", [](ClientData clientData, Tcl_Interp* interp, int argc, const char* argv[]) {
		std::string output = "[TCL] ";
		buildargstr(output, argc, argv);
		output += "\n";
		gEngfuncs.Con_Printf(output.c_str());
		return TCL_OK;
		}, nullptr, nullptr);
	Tcl_CreateCommand(s_pTclinterp, "clientcmd", [](ClientData clientData, Tcl_Interp* interp, int argc, const char* argv[]) {
		std::string cmds = "";
		buildargstr(cmds, argc, argv);
		gEngfuncs.pfnClientCmd(cmds.c_str());
		return TCL_OK;
		}, nullptr, nullptr);
	Tcl_CreateCommand(s_pTclinterp, "servercmd", [](ClientData clientData, Tcl_Interp* interp, int argc, const char* argv[]) {
		std::string cmds = "";
		buildargstr(cmds, argc, argv);
		gEngfuncs.pfnServerCmd(cmds.c_str());
		return TCL_OK;
		}, nullptr, nullptr);
	Tcl_CreateCommand(s_pTclinterp, "getcvar", [](ClientData clientData, Tcl_Interp* interp, int argc, const char* argv[]) {
		if (argc < 2) {
			char msg[] = "please send a cvar name as argument";
			Tcl_SetErrorCode(interp, "WE NEED A CVAR NAME AS ARGUMENT", NULL);
			Tcl_AddErrorInfo(interp, msg);
			return TCL_ERROR;
		}
		if (!gEngfuncs.pfnGetCvarPointer(argv[1])) {
			char msg[256] = {};
			snprintf(msg, 256, "no cvar named %s", argv[1]);
			Tcl_SetErrorCode(interp, "NO SUCH A CVAR", NULL);
			Tcl_AddErrorInfo(interp, msg);
			return TCL_ERROR;
		}
		char* cvar = gEngfuncs.pfnGetCvarString(argv[1]);
		Tcl_Obj* returnObj = Tcl_NewStringObj(cvar, strlen(cvar));
		Tcl_SetObjResult(s_pTclinterp, returnObj);
		return TCL_OK;
	}, nullptr, nullptr);
	Tcl_CreateCommand(s_pTclinterp, "gethudinfo", [](ClientData clientData, Tcl_Interp* interp, int argc, const char* argv[]) {
		if (argc < 2) {
			char msg[] = "please tell me whos info do you want?";
			Tcl_SetErrorCode(interp, "WE NEED A INFO INDEX AS ARGUMENT", NULL);
			Tcl_AddErrorInfo(interp, msg);
			return TCL_ERROR;
		}
		player_info_t* playerinfo = IEngineStudio.PlayerInfo(std::atoi(argv[1])-1);
		if (!playerinfo) {
			char msg[256] = {};
			snprintf(msg, 256, "no player as index %s", argv[1]);
			Tcl_SetErrorCode(interp, "NO SUCH A PLAYER", NULL);
			Tcl_AddErrorInfo(interp, msg);
			return TCL_ERROR;
		}
		Tcl_Obj* returnObj = Tcl_NewDictObj();
		Tcl_DictObjPut(s_pTclinterp, returnObj, Tcl_NewStringObj("name", -1), Tcl_NewStringObj(playerinfo->name, MAX_SCOREBOARDNAME));
		Tcl_DictObjPut(s_pTclinterp, returnObj, Tcl_NewStringObj("ping", -1), Tcl_NewIntObj(playerinfo->ping));
		Tcl_DictObjPut(s_pTclinterp, returnObj, Tcl_NewStringObj("loss", -1), Tcl_NewIntObj(playerinfo->packet_loss));
		Tcl_DictObjPut(s_pTclinterp, returnObj, Tcl_NewStringObj("model", -1), Tcl_NewStringObj(playerinfo->model, MAX_QPATH));
		Tcl_DictObjPut(s_pTclinterp, returnObj, Tcl_NewStringObj("topcolor", -1), Tcl_NewIntObj(playerinfo->topcolor));
		Tcl_DictObjPut(s_pTclinterp, returnObj, Tcl_NewStringObj("bottomcolor", -1), Tcl_NewIntObj(playerinfo->bottomcolor));
		Tcl_SetObjResult(s_pTclinterp, returnObj);
		return TCL_OK;
		}, nullptr, nullptr);
	Tcl_CreateCommand(s_pTclinterp, "getlocalinfo", [](ClientData clientData, Tcl_Interp* interp, int argc, const char* argv[]) {
		cl_entity_t* local = gEngfuncs.GetLocalPlayer();
		if (!local) {
			Tcl_SetErrorCode(interp, "NO SUCH A PLAYER", NULL);
			Tcl_AddErrorInfo(interp, "no local player for now");
			return TCL_ERROR;
		}
		Tcl_Obj* returnObj = Tcl_NewDictObj();
		Tcl_DictObjPut(s_pTclinterp, returnObj, Tcl_NewStringObj("index", -1), Tcl_NewIntObj(local->index));
		Tcl_Obj* origin[3] = { Tcl_NewDoubleObj(local->origin[0]), Tcl_NewDoubleObj(local->origin[1]) , Tcl_NewDoubleObj(local->origin[2]) };
		Tcl_DictObjPut(s_pTclinterp, returnObj, Tcl_NewStringObj("origin", -1), Tcl_NewListObj(3, origin));
		Tcl_Obj* angle[3] = { Tcl_NewDoubleObj(local->angles[0]), Tcl_NewDoubleObj(local->angles[1]) , Tcl_NewDoubleObj(local->angles[2]) };
		Tcl_DictObjPut(s_pTclinterp, returnObj, Tcl_NewStringObj("angles", -1), Tcl_NewListObj(3, angle));
		Tcl_Obj* velocity[3] = { Tcl_NewDoubleObj(local->curstate.velocity[0]), Tcl_NewDoubleObj(local->curstate.velocity[1]) , Tcl_NewDoubleObj(local->curstate.velocity[2]) };
		Tcl_DictObjPut(s_pTclinterp, returnObj, Tcl_NewStringObj("velocity", -1), Tcl_NewListObj(3, velocity));
		Tcl_SetObjResult(s_pTclinterp, returnObj);
		return TCL_OK;
		}, nullptr, nullptr);
	//registe command to tcl
	unsigned int hCmd = gEngfuncs.GetFirstCmdFunctionHandle();
	char szCmd[MAX_PATH];
	while (hCmd != NULL) {
		snprintf(szCmd, MAX_PATH, "gs_%s", gEngfuncs.GetCmdFunctionName(hCmd));
		Tcl_CreateCommand(s_pTclinterp, szCmd, [](ClientData clientData, Tcl_Interp* interp, int argc, const char* argv[]) {
			std::string args = (argv[0] + 3);
			args += " ";
			buildargstr(args, argc, argv);
			gEngfuncs.pfnClientCmd(args.c_str());
			return TCL_OK;
			}, nullptr, nullptr);
		hCmd = gEngfuncs.GetNextCmdFunctionHandle(hCmd);
	}
}
void IPluginsV4::LoadClient(cl_exportfuncs_t *pExportFunc){
	memcpy(&gExportfuncs, pExportFunc, sizeof(gExportfuncs));
	HMODULE hModule = GetModuleHandle("tcl86t.dll");
	if (!hModule) {
		g_pMetaHookAPI->SysError("[TCL] tlc86t.dll not load properly!");
		return;
	}
	CHAR path[MAX_PATH];
	GetModuleFileName(hModule, path, MAX_PATH);
	std::filesystem::path dllpath(path);
	std::string libpath = dllpath.parent_path().string();
	strncpy(s_szLibpath, (libpath + "\\lib" ).c_str(), MAX_PATH);
	strncpy(s_szPkgpath, (libpath + "\\pkg").c_str(), MAX_PATH);
	ResetTCLinter();
	pExportFunc->HUD_Init = [](){
		gExportfuncs.HUD_Init();
		gEngfuncs.pfnAddCommand("s_tcl_eval", []() {
			size_t argc = gEngfuncs.Cmd_Argc();
			if (argc < 2) {
				gEngfuncs.Con_Printf("s_tcl_eval [scripts]\n");
				return;
			}
			if (Tcl_Eval(s_pTclinterp, gEngfuncs.Cmd_Argv(1)) == TCL_ERROR) {
				const char* errorInfo = Tcl_GetVar(s_pTclinterp, "errorInfo", TCL_GLOBAL_ONLY);
				const char* errorCode = Tcl_GetVar(s_pTclinterp, "errorCode", TCL_GLOBAL_ONLY);
				gEngfuncs.Con_Printf("[TCL] Error when eval script, code: %s\n%s\n", errorCode, errorInfo);
			}	
		});
		gEngfuncs.pfnAddCommand("s_tcl_exec", []() {
			size_t argc = gEngfuncs.Cmd_Argc();
			if (argc < 2) {
				gEngfuncs.Con_Printf("s_tcl_exec [filepath]\n");
				return;
			}
			ResetTCLinter();
			char filepath[MAX_PATH];
			const char* file = gEngfuncs.Cmd_Argv(1);
			snprintf(filepath, MAX_PATH, "%s.tcl", file);
			if (!FILESYSTEM_ANY_FILEEXISTS(filepath))
				snprintf(filepath, MAX_PATH, "tcl/%s.tcl", file);
			if (!FILESYSTEM_ANY_FILEEXISTS(filepath)) {
				gEngfuncs.Con_Printf("[TCL] No such script in: %s.tcl, tcl/%s.tcl\n", file, file);
				return;
			}
			char local[MAX_PATH];
			FILESYSTEM_ANY_GETLOCALPATH(filepath, local, MAX_PATH);
			if (argc > 2) {
				for (size_t i = 2; i < argc; i++) {
					std::string tclargv = "set argv(" + std::to_string(i - 2) + ") ";
					tclargv += gEngfuncs.Cmd_Argv(i);
					tclargv += "\n";
					Tcl_Eval(s_pTclinterp, tclargv.c_str());
				}
				Tcl_SetVar(s_pTclinterp, "argc", std::to_string(argc - 2).c_str(), 0);
			}
			if (Tcl_EvalFile(s_pTclinterp, local) == TCL_ERROR) {
				const char* errorInfo = Tcl_GetVar(s_pTclinterp, "errorInfo", TCL_GLOBAL_ONLY);
				const char* errorCode = Tcl_GetVar(s_pTclinterp, "errorCode", TCL_GLOBAL_ONLY);
				gEngfuncs.Con_Printf("[TCL] Error executing script %s, code: %s\n%s\n", filepath, errorCode, errorInfo);
			}
				
		});
		gEngfuncs.pfnAddCommand("s_tcl_reset", []() {
			ResetTCLinter();
		});
	};
	pExportFunc->HUD_GetStudioModelInterface = [](int version, struct r_studio_interface_s** ppinterface, struct engine_studio_api_s* pstudio) -> int {
		memcpy(&IEngineStudio, pstudio, sizeof(IEngineStudio));
		return gExportfuncs.HUD_GetStudioModelInterface(version, ppinterface, pstudio);
	};
}
void IPluginsV4::ExitGame(int iResult){
	if (s_pTclinterp)
		Tcl_DeleteInterp(s_pTclinterp);
	Tcl_Finalize();
}
const char completeVersion[] ={
	BUILD_YEAR_CH0, BUILD_YEAR_CH1, BUILD_YEAR_CH2, BUILD_YEAR_CH3,
	'-',
	BUILD_MONTH_CH0, BUILD_MONTH_CH1,
	'-',
	BUILD_DAY_CH0, BUILD_DAY_CH1,
	'T',
	BUILD_HOUR_CH0, BUILD_HOUR_CH1,
	':',
	BUILD_MIN_CH0, BUILD_MIN_CH1,
	':',
	BUILD_SEC_CH0, BUILD_SEC_CH1,
	'\0'
};
const char *IPluginsV4::GetVersion(void){
	return completeVersion;
}
EXPOSE_SINGLE_INTERFACE(IPluginsV4, IPluginsV4, METAHOOK_PLUGIN_API_VERSION_V4);