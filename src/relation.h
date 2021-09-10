#ifndef RELATIONH
#define RELATIONH

#include <string>
#include <set>
#include <vector>
#include <iostream>

using namespace std;


struct prof {
  int _id=-1;
  string _name;
  string _email;
  std::set<string> _topics;
  std::vector<bool> _times;
  string _zoom_link;
  prof(int id, string name, string email, set<string> topics, vector<bool> times) :
    _id(id), _name(name), _email(email), _topics(topics), _times(times)
  {
  }

  int slots() {
    int s = 0;
    for(bool b : _times) {
      s+=b;
    }
    return s;
  }
};

struct student {
  int _id;
  string _name;
  string _email;
  string _univ;
  string _country;
  string _degree;
  string _grad_year;

  std::set<string> _topics;
  std::vector<prof*> _profs;
  student(int id, string name, string email, set<string> topics, vector<prof*> profs):
    _id(id), _name(name), _email(email), _topics(topics), _profs(profs)
  {}
};

#endif
