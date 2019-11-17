#include <bits/stdc++.h>

using namespace std;

int main() {
    string index, instruction;    
    string s;
    
    while(1){
		getline(cin, s);
		if(s == "00000000 <.data>:")
			break;
	}
	
    while(getline(cin, s)){
        stringstream ss;
        ss << s;
        ss >> index >> instruction;
		cout << index << " " << instruction << endl; 
    }
    	
	return 0;
}
