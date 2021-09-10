#ifndef TSVH
#define TSVH

#include <iostream>
#include <vector>
#include <set>
#include <sstream>
#include <algorithm> 
#include <cctype>
#include <locale>


using namespace std;

// trim from start (in place)
static inline void ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
}

// trim from end (in place)
static inline void rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), s.end());
}

// trim from both ends (in place)
static inline void trim(std::string &s) {
    ltrim(s);
    rtrim(s);
}

class TSVRow
{
  public:
    std::string_view operator[](std::size_t index) const
    {
      return std::string_view(&m_line[m_data[index] + 1], m_data[index + 1] -  (m_data[index] + 1));
    }
    std::size_t size() const
    {
      return m_data.size() - 1;
    }
    void readNextRow(std::istream& str)
    {
      std::getline(str, m_line);

      m_data.clear();
      m_data.emplace_back(-1);
      std::string::size_type pos = 0;
      while((pos = m_line.find('\t', pos)) != std::string::npos)
      {
        m_data.emplace_back(pos);
        ++pos;
      }
      // This checks for a trailing comma with no data after it.
      pos   = m_line.size();
      m_data.emplace_back(pos);
    }
  private:
    std::string         m_line;
    std::vector<int>    m_data;
};

std::istream& operator>>(std::istream& str, TSVRow& data)
{
  data.readNextRow(str);
  return str;
}   

void tokenize_string(string input, vector<string>& output) {
  std::stringstream ss(input);
  while(ss.good()) {
    string substr;
    getline( ss, substr, ',' );
    trim(substr);
    output.push_back( substr );
  }
}

void tokenize_string_to_set(string input, set<string>& output) {
    vector<string> topics;
    tokenize_string(input,topics);
    for(string i : topics) {
      output.insert(i);
    }

}

#endif
