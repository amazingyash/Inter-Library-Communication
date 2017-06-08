#include <iostream>
#include <fstream>

using namespace std;

int main()
{
  ifstream in;
  in.open("log.txt",ios::in | ios::app);
  long int d;
  long unsigned int sum = 0;
  while(in)
  {
    in>>d;
    sum = sum + d;
  }
  cout<<sum/100<<endl;
  in.close();
  return 0;
}
