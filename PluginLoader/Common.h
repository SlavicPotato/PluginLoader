#pragma once

#include "Debug.h"

namespace Common
{
	class ICommon
	{
	protected:
		ICommon() :
			m_refcount(0)
		{
		}

	private:
		virtual void __cdecl Release() {
			if (m_refcount == 0) {
				delete this;
			}
		}

		virtual void __cdecl IncRefCount() {
			InterlockedIncrement64(&m_refcount);
		}

		virtual void __cdecl DecRefCount() {
			InterlockedDecrement64(&m_refcount);
		}

		virtual uint64_t __cdecl GetRefCount() {
			return m_refcount;
		}

		volatile LONGLONG m_refcount;
	};
}
