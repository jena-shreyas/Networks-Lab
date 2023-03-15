// CPP program to demonstrate iterator
// invalidations
#include <bits/stdc++.h>
using namespace std;

// Driver Code
int main()
{

	// Creating a sample vector
	vector<int> v = { 1, 5, 10, 15, 20 };

	// Changing vector while iterating over it
	// (This causes iterator invalidation)
	for (auto it = v.begin(); it != v.end(); it++)
		if ((*it) == 5)
			v.push_back(-1);

	for (auto it = v.begin(); it != v.end(); it++)
		cout << (*it) << " ";

	return 0;
}
