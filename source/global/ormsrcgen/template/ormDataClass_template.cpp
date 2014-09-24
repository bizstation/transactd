
#pragma hdrstop
#include "%className%.h"
using namespace std;

% nameSpaceBegin %

    struct imple
{
    % datamemba % imple() : % menbaInit %
};

% className % :: % className % (void* owner)
    : m_impl(new imple()){

      }

      %
      className % :: % className % (const % className % &rt)
    : m_impl(new imple())
{
    *m_impl = *(rt.m_impl);
}

% className % ::~ % className % () { delete m_impl; }

% className % & % className % ::operator=(const % className % &rt)
{
    if (this != &rt)
    {
        *m_impl = *(rt.m_impl);
    }
    return *this;
}

% dataClassMembaFunc %

    % className % * % className % ::create(void* owner)
{
    return new % className % (owner);
}

% className % *create(% className % _ptr_list & mdls, int)
{
    return % className % ::create(&mdls);
}

% nameSpaceEnd %
