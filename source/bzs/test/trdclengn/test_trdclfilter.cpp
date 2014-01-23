//---------------------------------------------------------------------------

#pragma hdrstop

//#include "test_trdclfilter.h"



//---------------------------------------------------------------------------
#pragma package(smart_init)


void testResultField()
{
    BOOST_CHECK_MESSAGE((1 == 1), "GetEqual id 1");

}

// ------------------------------------------------------------------------
BOOST_AUTO_TEST_SUITE(filter)
    BOOST_FIXTURE_TEST_CASE(resultField, fixture) {testResultField();}
BOOST_AUTO_TEST_SUITE_END()
