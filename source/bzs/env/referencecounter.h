#include <vector>
#include <boost/thread/mutex.hpp>

struct referencePtrPair
{
	void* referer;
	void* referred_ptr;
};

class referenceCounter
{
	boost::mutex m_mutex;

public:
	std::vector<referencePtrPair> ptrs;

	inline void add(void* referer, void* referred_ptr)
	{
		boost::mutex::scoped_lock lck(m_mutex);
		referencePtrPair v = {referer, referred_ptr};
		ptrs.push_back(v);
	}

	inline void remove(void* referer)
	{
		boost::mutex::scoped_lock lck(m_mutex);
		for (size_t i = 0; i < ptrs.size(); ++i)
		{
			if (ptrs[i].referer == referer)
				ptrs.erase(ptrs.begin()+i);
		}
	}

	inline int getReferenceCount(void* referred_ptr)
	{
		boost::mutex::scoped_lock lck(m_mutex);
		int cnt = 0;
		for (size_t i = 0; i < ptrs.size(); ++i)
		{
			if (ptrs[i].referred_ptr == referred_ptr)
				cnt++;
		}
		return cnt;
	}

#ifdef SWIGRUBY
	inline void mark()
	{
		boost::mutex::scoped_lock lck(m_mutex);
		for (size_t i = 0; i < ptrs.size(); ++i)
		{
			VALUE object = SWIG_RubyInstanceFor(ptrs[i].referred_ptr);
			if (object != Qnil)
				rb_gc_mark(object);
		}
	}
#endif
};
