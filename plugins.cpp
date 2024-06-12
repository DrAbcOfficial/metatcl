#include <metahook.h>
#include <string>
#include <filesystem>
#include <tcl.h>

#pragma comment(lib,"tcl86t.lib")

cl_exportfuncs_t gExportfuncs = { 0 };
mh_interface_t* g_pInterface = nullptr;
metahook_api_t* g_pMetaHookAPI = nullptr;
IFileSystem* g_pFileSystem = nullptr;
cl_enginefunc_t gEngfuncs;

static Tcl_Interp* s_pTclinterp = nullptr;

void IPluginsV4::Init(metahook_api_t *pAPI, mh_interface_t *pInterface, mh_enginesave_t *pSave){
	g_pInterface = pInterface;
	g_pMetaHookAPI = pAPI;
}
void IPluginsV4::Shutdown(void){
}
void IPluginsV4::LoadEngine(cl_enginefunc_t *pEngfuncs){
	g_pFileSystem = g_pInterface->FileSystem;
	memcpy(&gEngfuncs, pEngfuncs, sizeof(gEngfuncs));
}
void IPluginsV4::LoadClient(cl_exportfuncs_t *pExportFunc){
	memcpy(&gExportfuncs, pExportFunc, sizeof(gExportfuncs));

	s_pTclinterp = Tcl_CreateInterp();
	HMODULE hModule = GetModuleHandle("tcl86t.dll");
	if (!hModule) {
		g_pMetaHookAPI->SysError("[TCL] tlc86t.dll not load properly!");
		return;
	}
	CHAR path[MAX_PATH];
	GetModuleFileName(hModule, path, MAX_PATH);
	std::filesystem::path dllpath(path);
	std::string libpath = dllpath.parent_path().string() + "\\lib";

	Tcl_SetVar(s_pTclinterp, "tcl_library", libpath.c_str(), TCL_GLOBAL_ONLY);
	if (Tcl_Init(s_pTclinterp) == TCL_ERROR) {
		g_pMetaHookAPI->SysError("[TCL] Tcl init failed!\n%s", Tcl_GetStringResult(s_pTclinterp));
		return;
	}
	pExportFunc->HUD_Init = []() {
		gExportfuncs.HUD_Init();
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
		gEngfuncs.pfnAddCommand("tcl_eval", []() {
			size_t argc = gEngfuncs.Cmd_Argc();
			if (argc < 2) {
				gEngfuncs.Con_Printf("tcl_eval [scripts]\n");
				return;
			}
			if (Tcl_Eval(s_pTclinterp, gEngfuncs.Cmd_Argv(1)) == TCL_ERROR)
				gEngfuncs.Con_Printf("[TCL] Error when eval script: %s\n", Tcl_GetStringResult(s_pTclinterp));
		});
		gEngfuncs.pfnAddCommand("tcl_exec", []() {
			size_t argc = gEngfuncs.Cmd_Argc();
			if (argc < 2) {
				gEngfuncs.Con_Printf("tcl_script [filepath]\n");
				return;
			}
			char filepath[MAX_PATH];
			const char* file = gEngfuncs.Cmd_Argv(1);
			snprintf(filepath, MAX_PATH, "%s.tcl", file);

			FileHandle_t script = g_pFileSystem->Open(filepath, "r");
			if (!script) {
				snprintf(filepath, MAX_PATH, "tcl/%s.tcl", file);
				script = g_pFileSystem->Open(filepath, "r");
			}
			if (!script) {
				gEngfuncs.Con_Printf("[TCL] No such script in: %s.tcl, tcl/%s.tcl\n", file, file);
				return;
			}
			size_t filesize = g_pFileSystem->Size(script);
			char* buffer = new char[filesize + 1];
			g_pFileSystem->Read(buffer, filesize, script);
			g_pFileSystem->Close(script);
			buffer[filesize] = '\0';
			if (Tcl_Eval(s_pTclinterp, buffer) == TCL_ERROR)
				gEngfuncs.Con_Printf("[TCL] Error when executed script: %s\n", Tcl_GetStringResult(s_pTclinterp));
			delete[] buffer;
		});
	};
}
void IPluginsV4::ExitGame(int iResult){
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