#pragma once
#ifdef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif

#include "../../Enums/Enums.hpp"
#include "../../FwdDecls/FwdDecls.hpp"
#include <d3d9.h>
#include <d3d11.h>
#include <Windows.h>
#include <string>
#include <tuple>

#pragma pack(push, 1)

// can we escape this template hell
class YYTKEventBase
{
public:
	virtual EventType GetEventType() const = 0;
};

template <typename _ReturnValue, typename _Function, EventType _Event, typename... _FunctionArgs>
class YYTKEvent : public YYTKEventBase
{
protected:
	std::tuple<_FunctionArgs...> s_tArguments;
	_ReturnValue s_tReturnValue;
	_Function s_tOriginal;
	bool s_CalledOriginal;

public:
	_ReturnValue& Call(_FunctionArgs... Args)
	{
		s_tReturnValue = s_tOriginal(Args...);
		s_CalledOriginal = true;
		return s_tReturnValue;
	}

	// Warning: This function does NOT set any internal flags, use Call() to call original instead of this!
	_Function Function() const
	{
		return s_tOriginal;
	}

	std::tuple<_FunctionArgs...>& Arguments()
	{
		return s_tArguments;
	}

	bool& CalledOriginal()
	{
		return s_CalledOriginal;
	}

	_ReturnValue& GetReturn()
	{
		return s_tReturnValue;
	}

	void Cancel(const _ReturnValue& newReturn)
	{
		this->CalledOriginal() = true;
		this->GetReturn() = newReturn;
	}

	virtual EventType GetEventType() const override
	{
		return _Event;
	}

	YYTKEvent(_Function Original, _FunctionArgs... Args)
	{
		this->s_CalledOriginal = false;
		this->s_tReturnValue = 0; // Might be UB, who knows
		this->s_tArguments = std::make_tuple(Args...);
		this->s_tOriginal = Original;
	}

	YYTKEvent(const std::string& InternalName, _Function Original, _FunctionArgs... Args)
	{
		this->s_CalledOriginal = false;
		this->s_tReturnValue = 0; // Might be UB, who knows
		this->s_tArguments = std::make_tuple(Args...);
		this->s_tOriginal = Original;
	}
};

template <typename _Function, EventType _Event, typename... _FunctionArgs>
class YYTKEvent<void, _Function, _Event, _FunctionArgs...> : public YYTKEventBase // Specialization for void return type
{
protected:
	std::tuple<_FunctionArgs...> s_tArguments;
	_Function s_tOriginal;
	bool s_CalledOriginal;

public:
	void Call(_FunctionArgs... Args)
	{
		s_tOriginal(Args...);
		s_CalledOriginal = true;
	}

	// Warning: This function does NOT set any internal flags, use Call() to call original instead of this!
	_Function Function() const
	{
		return s_tOriginal;
	}

	std::tuple<_FunctionArgs...>& Arguments()
	{
		return s_tArguments;
	}

	bool& CalledOriginal()
	{
		return s_CalledOriginal;
	}

	virtual EventType GetEventType() const override
	{
		return _Event;
	}

	YYTKEvent(_Function Original, _FunctionArgs... Args)
	{
		this->s_CalledOriginal = false;
		this->s_tArguments = std::make_tuple(Args...);
		this->s_tOriginal = Original;
	}
};

using YYTKCodeEvent = YYTKEvent<bool, bool(__cdecl*)(CInstance*, CInstance*, CCode*, RValue*, int), EventType::EVT_CODE_EXECUTE, CInstance*, CInstance*, CCode*, RValue*, int>;
using YYTKEndSceneEvent = YYTKEvent<HRESULT, HRESULT(__stdcall*)(LPDIRECT3DDEVICE9), EventType::EVT_ENDSCENE, LPDIRECT3DDEVICE9>;
using YYTKMessageBoxEvent = YYTKEvent<int, int(__stdcall*)(HWND, LPCWSTR, LPCWSTR, UINT), EventType::EVT_MESSAGEBOX, HWND, LPCWSTR, LPCWSTR, UINT>;
using YYTKPresentEvent = YYTKEvent<HRESULT, HRESULT(__stdcall*)(IDXGISwapChain*, UINT, UINT), EventType::EVT_PRESENT, IDXGISwapChain*, UINT, UINT>;
using YYTKWindowProcEvent = YYTKEvent<LRESULT, LRESULT(__stdcall*)(HWND, UINT, WPARAM, LPARAM), EventType::EVT_WNDPROC, HWND, UINT, WPARAM, LPARAM>;
using YYTKResizeBuffersEvent = YYTKEvent<HRESULT, HRESULT(__stdcall*)(IDXGISwapChain*, UINT, UINT, UINT, DXGI_FORMAT, UINT), EventType::EVT_RESIZEBUFFERS, IDXGISwapChain*, UINT, UINT, UINT, DXGI_FORMAT, UINT>;
using YYTKErrorEvent = YYTKEvent<void, void(__cdecl*)(const char*, ...), EventType::EVT_YYERROR, const char*>; // template specializations ftw
using YYTKScriptEvent = YYTKEvent<char*, char* (__cdecl*)(CScript*, int, char*, VMExec*, YYObjectBase*, YYObjectBase*), EventType::EVT_DOCALLSCRIPT, CScript*, int, char*, VMExec*, YYObjectBase*, YYObjectBase*>;

#pragma pack(pop)