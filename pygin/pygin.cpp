#include "headers.hpp"

#include "pygin.hpp"
#include "module.hpp"

#include "py_dictionary.hpp"
#include "py_err.hpp"
#include "py_import.hpp"
#include "py_list.hpp"
#include "py_string.hpp"
#include "py_sys.hpp"
#include "py_tools.hpp"

static void add_to_python_path(const std::wstring& Path)
{
	py::list PathList(py::sys::get_object("path"));
	const py::string NewItem(Path);
	bool Found = false;

	for (size_t i = 0, size = PathList.size();  i != size; ++i)
	{
		if (!NewItem.compare(PathList[i]))
		{
			Found = true;
		}
	}
	if (!Found)
	{
		PathList.push_back(NewItem);
	}
}

static py::object add_or_reload_module(const std::wstring& Name)
{
	py::dictionary ModulesDict(py::sys::get_object("modules"));
	const py::string NewModuleName(Name);
	const auto ExistingModule = ModulesDict.get_at(NewModuleName);
	auto NewModule = py::import::import(NewModuleName);

	if (ExistingModule)
	{
		NewModule = py::import::reload_module(ExistingModule);
	}
	return NewModule;
}

extern "C" IMAGE_DOS_HEADER __ImageBase;

pygin::pygin(GlobalInfo* Info)
{
	Info->StructSize = sizeof(GlobalInfo);

	Info->MinFarVersion = MAKEFARVERSION(3, 0, 0, 3000, VS_RELEASE);
	Info->Version = MAKEFARVERSION(0, 0, 0, 3, VS_ALPHA);

	static const UUID PyginID = { 0x66714600, 0xd5fa, 0x4fae, { 0xac, 0x4b, 0x77, 0x25, 0xe5, 0x7a, 0x87, 0x39 } };
	Info->Guid = PyginID;

	Info->Title = L"Pygin";
	Info->Author = L"Far Group";
	Info->Description = L"Python language support for Far Manager";

	py::initialize();

	wchar_t AdaptherPath[MAX_PATH];
	GetModuleFileNameW(reinterpret_cast<HINSTANCE>(&__ImageBase), AdaptherPath, static_cast<DWORD>(std::size(AdaptherPath)));
	*(wcsrchr(AdaptherPath, L'\\') + 1) = 0;

	m_PyginModule = create_module((AdaptherPath + L"pygin\\__init__.py"s).data());
}

pygin::~pygin()
{
	m_PyginModule.reset();
	py::finalize();
}

bool pygin::is_module(const wchar_t* FileName) const
{
	// BUGBUG, ends_with
	static const auto Suffix = L"\\__init__.py";
	static const auto SuffixSize = wcslen(Suffix);

	const auto FileNameLength = wcslen(FileName);
	return FileNameLength >= SuffixSize && !_wcsnicmp(FileName + FileNameLength - SuffixSize, Suffix, SuffixSize);
}

std::unique_ptr<module> pygin::create_module(const wchar_t* FileName)
{
	std::wstring Dir(FileName);
	const auto SlashPos = Dir.rfind(L'\\');
	Dir.resize(SlashPos);
	const auto PrevSlashPos = Dir.rfind(L'\\');
	const std::wstring Path(Dir, 0, PrevSlashPos);
	const std::wstring ModuleName(Dir, PrevSlashPos + 1);
	add_to_python_path(Path);
	const auto Object = add_or_reload_module(ModuleName);
	py::err::print_if_any();
	return Object? std::make_unique<module>(Object) : nullptr;
}

FARPROC WINAPI pygin::get_function(HANDLE Instance, const wchar_t* FunctionName)
{
	static std::unordered_map<std::wstring, void*> FunctionsMap =
	{
#define KEY_VALUE(x) { L ## #x, x }

		KEY_VALUE(GetGlobalInfoW),
		KEY_VALUE(SetStartupInfoW),
		KEY_VALUE(OpenW),
		KEY_VALUE(ClosePanelW),
		KEY_VALUE(GetPluginInfoW),
		KEY_VALUE(GetOpenPanelInfoW),
		KEY_VALUE(GetFindDataW),
		KEY_VALUE(FreeFindDataW),
		KEY_VALUE(SetDirectoryW),
		KEY_VALUE(GetFilesW),
		KEY_VALUE(PutFilesW),
		KEY_VALUE(DeleteFilesW),
		KEY_VALUE(MakeDirectoryW),
		KEY_VALUE(ProcessHostFileW),
		KEY_VALUE(SetFindListW),
		KEY_VALUE(ConfigureW),
		KEY_VALUE(ExitFARW),
		KEY_VALUE(ProcessPanelInputW),
		KEY_VALUE(ProcessPanelEventW),
		KEY_VALUE(ProcessEditorEventW),
		KEY_VALUE(CompareW),
		KEY_VALUE(ProcessEditorInputW),
		KEY_VALUE(ProcessViewerEventW),
		KEY_VALUE(ProcessDialogEventW),
		KEY_VALUE(ProcessSynchroEventW),
		KEY_VALUE(ProcessConsoleInputW),
		KEY_VALUE(AnalyseW),
		KEY_VALUE(CloseAnalyseW),
		KEY_VALUE(GetContentFieldsW),
		KEY_VALUE(GetContentDataW),
		KEY_VALUE(FreeContentDataW),

#undef KEY_VALUE
	};

	const auto Module = static_cast<module*>(Instance);
	return Module->check_function(FunctionName) && FunctionsMap.count(FunctionName)? reinterpret_cast<FARPROC>(FunctionsMap[FunctionName]) : nullptr;
}
