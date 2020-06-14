#pragma once

class ICommon
{
protected:
	virtual void __cdecl Release() = 0;
	virtual void __cdecl IncRefCount() = 0;
	virtual void __cdecl DecRefCount() = 0;
public:
	virtual uint64_t __cdecl GetRefCount() = 0;
};

template <class T>
class IScoped
{
	static_assert(std::is_base_of<ICommon, T>::value, "Not derived from ICommon");
public:
	IScoped() :
		m_cl(nullptr)
	{
	}

	IScoped(T* cl)
		: m_cl(cl)
	{
		m_cl->IncRefCount();
	}

	virtual ~IScoped() noexcept
	{
		if (m_cl != nullptr)
		{
			try
			{
				m_cl->DecRefCount(); 
				m_cl->Release();
			}
			catch (...)
			{
			}
		}
	}

	T* operator->() const
	{
		return m_cl;
	}

	operator T* () const
	{
		return m_cl;
	}

	T* Get() const
	{
		return m_cl;
	}

	IScoped(IScoped<T>& o) { copy(o); };
	IScoped(const IScoped<T>&& o) { copy(o); };
	void operator=(IScoped<T>& o) { copy(o); };
	void operator=(const IScoped<T>&& o) { copy(o); };

private:

	void copy(IScoped<T>& o)
	{
		m_cl = o.m_cl;
		m_cl->IncRefCount();
		//o.m_cl = nullptr;
	}

	T* m_cl;
};