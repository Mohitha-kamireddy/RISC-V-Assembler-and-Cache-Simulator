#ifndef ASSEMBLER_HPP
#define ASSEMBLER_HPP
#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <map>
#include <sstream>
using namespace std;
class Assembler{
public:
class FuncTypes {
public:
map<string,int> R, I,B,J,S;
};

void up(string &s);
int toInt(const string &s);
string toBin(int num);
string immBin(string imm, int bits);
string bin5(int n);

void makeMaps(FuncTypes &F);

string rtype(vector<string> tokens, FuncTypes &F);
string itype(vector<string> tokens, FuncTypes &F);
string btype(vector<string> tokens, FuncTypes &F);
string jtype(vector<string> tokens, FuncTypes &F);
string stype(vector<string> tokens, FuncTypes &F);
string parse(vector<string> tokens, FuncTypes &F);

vector<string> Tokenize(const string &line);

vector<string> assemble();
long binToDec(const string &bin);
string decToBin(long dec);
void singlecycleprocessor(const vector<string> &binaries);

} ;

#endif