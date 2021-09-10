#include <vector>


#include <iterator>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <string>
#include <utility>
#include "tsv.h"
#include "assignment.h"
#include "relation.h"

using namespace std;

std::map<string,int> topic_weight;

// List of professors and students
std::vector<prof> profs;
std::vector<student> students;

// I use this to be able to convert student/prof names into pointers to the structure.
// Is it lazy coding: yes.
std::map<string,student*> student_by_name;
std::map<string,int> prof_by_name;

// For each pair of (student,prof), this assigns a score indicating the value of the match
std::map<std::pair<int,int>,int> pref_score;

// I set up different topic weights depending on how specific I thought different
// interests were, and how common they were.  Uncommon and more narrow interests (at
// the current moment) got more weight)
void setup_topic_weight() {
  topic_weight["Cloud Data-centers Internet Services"]=4;
  topic_weight["Storage and Compute of Big-Data"]=4;
  topic_weight["Parallel GPU Multicore Architectures"]=3;
  topic_weight["Accelerators & Heterogenous Architectures"]=3;
  topic_weight["Programming Models Languages and Compilers"]=4;
  topic_weight["CPUs and Microarchitecture"]=4;
  topic_weight["Embedded IoT and Edge"]=6;
  topic_weight["Security & Privacy"]=5;
  topic_weight["Reliability & Availability"]=6;
  topic_weight["Virtualization"]=8;
  topic_weight["Memory Systems"]=3;
  topic_weight["Simulation Measurement & Modeling"]=4;
  topic_weight["Power Energy & Thermals"]=5;
  topic_weight["Testing & Veriﬁcation"]=8;
  topic_weight["Approximate Computing"]=6;
  topic_weight["Quantum Computing"]=8;
  topic_weight["Neuromorphic Computing"]=8;
  topic_weight["Machine Learning"]=4;
  topic_weight["Bio-inspired Computing & Storage"]=4;
  topic_weight["Graph Processing"]=5;
  topic_weight["Reconfigurable Computing"]=4;
  topic_weight["FPGAs"]=2;
  topic_weight["Interconnection Networks"]=0;
  topic_weight["NoCs"]=0;
  topic_weight["Routing Algorithms"]=0;
  topic_weight["Green Computing"]=0;
  topic_weight["Sustainability"]=0;


}

int main(int argc, char** argv) {
  setup_topic_weight();

  cout << "Parsing Prof TSV File ---- \n";

  std::ifstream       if_profs("profs.tsv");
  TSVRow              row;
  if_profs>>row; //just one empty row
  int prof_id=0;
  while(if_profs >> row) {
    // These lines define the format of the TSV file
    std::string prof_name = static_cast<std::string>(row[1]);
    std::string prof_email = static_cast<std::string>(row[2]);
    std::string prof_time_string = static_cast<std::string>(row[3]);
    std::string prof_topics_string = static_cast<std::string>(row[5]);
    std::string zoom_link = static_cast<std::string>(row[7]);

    // Times is a vector of boolean values indicating whether a professor
    // can make that meeting time.  This is very hard-coded ATM, and should
    // be generalized.  Currently we assume 6 slots.
    vector<bool> times;
    if(prof_time_string == "11am-12pm EDT, 2pm-3pm EDT") {
      times = {1,1,1,1,1,1};
    } else if(prof_time_string == "11am-12pm EDT") {
      times = {1,1,1,0,0,0};
    } else if(prof_time_string == "2pm-3pm EDT") {
      times = {0,0,0,1,1,1};
    } else if(prof_time_string == "None") {
      times = {0,0,0,0,0,0};
    } else {
      cout << "Bad time string:\n";
      cout << prof_time_string << "\n";
      cout << prof_name << "\n";
      assert(0);
    }

    set<string> topic_set;
    tokenize_string_to_set(prof_topics_string,topic_set);

    //just check we have a score
    for(string s : topic_set) {
      if(topic_weight.count(s)==0) {
        cout << "bad prof topic: " << s << "\n";
        assert(0);
      }
    }

    if(!(topic_set.size()<=6 && topic_set.size()>0)) {
      cout << "bad topic set for: " << prof_name << "\n";
      for(string s : topic_set) {
        cout << s << "|";
      }
      cout << "\n";
      assert(0);
    }


    // Add a professor to the list of professors
    prof_by_name[prof_name]=prof_id;
    profs.emplace_back(prof_id++,prof_name,prof_email, topic_set,times);
    
    //oops, add the zoom link
    prof& p = profs.back();
    p._zoom_link = zoom_link;

    //cout << "\"" << prof_name << "\" assigned to " << prof_id-1 << "\n";

  }

  cout << "FINISHED Parsing Professor TSV File ---- \n";


  cout << "Parsing Student TSV File ---- \n";

  std::ifstream       if_students("students.tsv");
  if_students>>row;
  int student_id=0;
  int total_students=0;
  while(if_students >> row) {
    ++total_students;
    
    // this defines the columns of the TSV files
    std::string student_name = static_cast<std::string>(row[2]);
    std::string student_email = static_cast<std::string>(row[1]);
    std::string student_coming = static_cast<std::string>(row[3]);
    std::string student_topics_string = static_cast<std::string>(row[4]);

    std::string univ = static_cast<std::string>(row[11]);
    std::string country = static_cast<std::string>(row[12]);
    std::string degree = static_cast<std::string>(row[13]);
    std::string grad_year = static_cast<std::string>(row[14]);

    if(student_coming=="No") continue;
    else if (student_coming=="Yes") {}
    else {
      cout << "Weird input for invitation\n";
      cout << student_coming << "\n";
      assert(0);
    }

    set<string> topic_set;
    tokenize_string_to_set(student_topics_string,topic_set);


    //Determine the preferences for each student/prof pair, BASED ON THE PROFESSOR LIST
    //This implementation assigns a score of "20" to the first
    //prof, then "18" to the second prof, and so on.
    std::vector<prof*> prof_prefs;
    for(int i = 0, prof_pref=20; i < 6; ++i, prof_pref-=2) {
      string prof_name = static_cast<std::string>(row[5+i]);
      trim(prof_name);
      cout << "Prof: \"" << prof_name << "\" : ";

      if(prof_name!="" && prof_name!="I'm not sure, please choose for me.") {
        if(prof_by_name.count(prof_name)==0) {
          cout << "Prof: \"" << prof_name << "\" could not be found\n";
          cout << "Student: " << student_name <<"\n";
          assert(0);
        }

        assert(prof_by_name.count(prof_name));

        int pid = prof_by_name[prof_name];
        prof& p = profs[pid];
        assert(p._id == pid);

        // Store the vector of profs for each student -- this doesn't get used though, can delete
        prof_prefs.push_back(&p);

        // This is where the preferences for each pair of student/prof are recorded
        pref_score[std::make_pair(student_id,p._id)]=prof_pref;
        cout << prof_pref << " prof_id:" << p._id << "\n";
      }
    }

    students.emplace_back(student_id++,student_name,student_email,topic_set,prof_prefs);
    student& s = students.back();
    s._univ=univ;
    s._country=country;
    s._degree=degree;
    s._grad_year=grad_year;

    if(topic_set.size()==0) {
      cout << student_name << " didn't select any topics...\n";
      continue;
    }
    assert(topic_set.size()<=4);

    //compute match topic match
    int max_topic_score=0;
    for (string topic_string : topic_set) {
      if(topic_weight.count(topic_string)==0) {
        cout << "bad student topic: " << topic_string << "\n";
        assert(0);
      }
      max_topic_score+=topic_weight[topic_string];
    }

    if(max_topic_score==0) {
      cout << student_name << " topic score is zero\n";
      assert(0);
    }
       
    //Determine the preferences for each student/prof pair, BASED ON THE TOPICS
    for (prof& p : profs) {
      std::pair<int,int> match_pair = std::make_pair(s._id,p._id);

      int cur_score=0;
      bool found_match=false;;
      for (string topic_string : topic_set) {
        if(p._topics.count(topic_string)) {
          cur_score+=topic_weight[topic_string];
          found_match=true;
        }
      }

      // This is some funky math that I decided I liked.  Basically it calculates how
      // "similar" two lists of preferences are, weighted by the topic weights.
      // This could be changed or made simpler...
      float match_ratio=(float)cur_score/(float)max_topic_score;
      int score=(found_match?5:0)+match_ratio*8.0; //this number is chosen to balance
                                                   //topic vs prof-list weight

      //overall score is the max of manual score and topic-based score
      pref_score[match_pair]=std::max((int)score,pref_score[match_pair]);
    }

  }

  cout << "FINISHED Parsing Student TSV File ---- \n";

  // This code slightly penalizes arbitrary meetings .. this may be deleted
  for (unsigned s = 0; s < students.size(); ++s) {
    printf("%22s ",students[s]._name.c_str());

    for (unsigned p = 0; p < profs.size(); ++p) {
      pair<int,int> id_pair = std::make_pair(s,p);
      if(pref_score[id_pair]==0) {
        pref_score[id_pair]=-2;
      } else {
        //cout << p << "-" << s << "=" << pref_score[id_pair] << "\n";
      }
      printf("%3d ",pref_score[id_pair]);
    }
    printf("\n");
  }
 
  //Call the mixed-integer linear program
  AssignmentMip(profs,students,pref_score);

  return EXIT_SUCCESS;
}
