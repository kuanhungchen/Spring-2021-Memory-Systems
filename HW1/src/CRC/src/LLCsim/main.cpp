#include <iostream>
using namespace std;

int main()
{
    cout << "Test creating an CACHE_REPLACEMENT_STATE object" << endl;
    CACHE_REPLACEMENT_STATE * cacheReplState = new CACHE_REPLACEMENT_STATE( 4, 4, 0 );


    return 0;
}


