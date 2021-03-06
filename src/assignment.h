#ifndef ASSIGNH
#define ASSIGNH

#include "ortools/base/logging.h"
#include "ortools/linear_solver/linear_solver.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <string>
#include <utility>

#include "relation.h"

using namespace operations_research;

void AssignmentMip(std::vector<prof> profs, std::vector<student> students, std::map<std::pair<int,int>,int> pref_score) {
  MPSolver solver("assignment_mip", MPSolver::SCIP_MIXED_INTEGER_PROGRAMMING);

  int num_profs = profs.size();
  int num_students = students.size();
  int num_times = 6;

  // Variables
  // Mapping of profs to students
  vector<vector<vector<const MPVariable*>>> Mpts(
      num_profs, vector<vector<const MPVariable*>>(
      num_times, vector<const MPVariable*>(num_students)));
  for (int p = 0; p < num_profs; ++p) {
    for (int t = 0; t < num_times; ++t) {
      for (int s = 0; s < num_students; ++s) {
        Mpts[p][t][s] = solver.MakeIntVar(0, 1, "");
      }
    }
  }

  const MPVariable* Tmin;
  Tmin = solver.MakeIntVar(0,100,"");

  //This is not currently used
  vector<const MPVariable*> MG4(num_students);
  for (int s = 0; s < num_students; ++s) {
    MG4[s] = solver.MakeIntVar(0,1, "");
  }

  // Total Per Student
  //std::vector<const MPVariable*> Ts(num_students);
  //for (int s = 0; s < num_students; ++s) {
  //  Ts[s] = solver.MakeIntVar(0,100,"");
  //}

  
  // Constraints

  // Each prof is only available in some times
  for (int p = 0; p < num_profs; ++p) {
    for (int t = 0; t < num_times; ++t) {
      if(profs[p]._times[t]==0) {
        for (int s = 0; s < num_students; ++s) {
          LinearExpr prof_slot_student = Mpts[p][t][s];
          solver.MakeRowConstraint(prof_slot_student == 0.0);
        }
      }
    }
  }

  // Each prof/slot is assigned to exactly 2 students
  // This can be changed to implement different constraints, 
  // like each prof meets less than a certain number, etc.
  for (int p = 0; p < num_profs; ++p) {
    for (int t = 0; t < num_times; ++t) {
      LinearExpr prof_sum;
      for (int s = 0; s < num_students; ++s) {
        prof_sum += Mpts[p][t][s];
      }
      //solver.MakeRowConstraint(prof_sum <= 3.0);

      if(profs[p]._times[t]) {
        solver.MakeRowConstraint(prof_sum == 2.0);
      }
    }
  }

  // This was something funky for balancing the number of students across
  // profs.  I turned it off because all profs meet with 2 students exactly
  // in this version.
  for (int p = 0; p < num_profs; ++p) {
    LinearExpr prof_sum;
    for (int t = 0; t < num_times; ++t) {
      for (int s = 0; s < num_students; ++s) {
        prof_sum += Mpts[p][t][s];
      }
    }

    //solver.MakeRowConstraint(prof_sum >= (int)(1.8 *profs[p].slots()));
    //solver.MakeRowConstraint(prof_sum <= (int)(2.4 *profs[p].slots()));
  }


  // No student should go to multiple profs during the same slot
  for (int s = 0; s < num_students; ++s) {
    for (int t = 0; t < num_times; ++t) {
      LinearExpr student_slot_sum;
      for (int p = 0; p < num_profs; ++p) {
         student_slot_sum += Mpts[p][t][s];
      }
      solver.MakeRowConstraint(student_slot_sum <= 1.0);
    }
  }

  // No student should go to the same prof twice
  for (int s = 0; s < num_students; ++s) {
    for (int p = 0; p < num_profs; ++p) {
      LinearExpr student_prof_sum;
      for (int t = 0; t < num_times; ++t) {
         student_prof_sum += Mpts[p][t][s];
      }
      solver.MakeRowConstraint(student_prof_sum <= 1.0);
    }
  }

  // Minimum number of meetings assigned per student
  for (int s = 0; s < num_students; ++s) {
    LinearExpr total_student_sum;
   
    LinearExpr student_sum;
    for (int p = 0; p < num_profs; ++p) {
      for (int t = 0; t < 3; ++t) {
        student_sum += Mpts[p][t][s];
        total_student_sum += Mpts[p][t][s];
      }
    }
    // You could set a minimum number of meetings here if you want
    //solver.MakeRowConstraint(student_sum >= Tmin);
    //solver.MakeRowConstraint(student_sum >=2);


    LinearExpr student_sum_2;
    for (int p = 0; p < num_profs; ++p) {
      for (int t = 3; t < 6; ++t) {
        student_sum_2 += Mpts[p][t][s];
        total_student_sum += Mpts[p][t][s];
      }
    }
    // You could set a minimum number of meetings here if you want
    //solver.MakeRowConstraint(student_sum >= Tmin);
    //solver.MakeRowConstraint(student_sum_2 >= 2);

    solver.MakeRowConstraint(total_student_sum >= Tmin);

    solver.MakeRowConstraint(total_student_sum <= 5);


    // Also, give a penalty if we assign 4 meetings
    //LinearExpr student_sum_bias;

    //student_sum_bias += student_sum;
    //student_sum_bias -= (num_times-1);

    //solver.MakeRowConstraint(student_sum_bias <= MG4[s]);
  }


  // Objective: maximize the sum of the preference score
  MPObjective* const objective = solver.MutableObjective();
  for (int p = 0; p < num_profs; ++p) {
    for (int s = 0; s < num_students; ++s) {
      for (int t = 0; t < num_times; ++t) {
        //if(p==9 && t==0) {
        //  cout << students[s] << " prefers " << profs[p] 
        //       << " score " << pref_score[make_pair(s,p)] << "\n";
        //}
        objective->SetCoefficient(Mpts[p][t][s], pref_score[make_pair(s,p)]);
      }
    }
  }

  //Also want to give a bonus to balancing minimum meetings per student
  objective->SetCoefficient(Tmin,200);

  //for (int s = 0; s < num_students; ++s) {
  //  objective->SetCoefficient(MG4[s],-2);
  //}

  objective->SetMaximization();

  cout << "Calling Solve!\n";

  //comment this if you don't want to see solver output
  solver.EnableOutput(); 

  //you can also set a solver time limit
  //solver.set_time_limit(1);


  // Solve
  const MPSolver::ResultStatus result_status = solver.Solve();

  // Print solution.
  // Check that the problem has a feasible solution.
  if (result_status != MPSolver::OPTIMAL &&
      result_status != MPSolver::FEASIBLE) {
    LOG(FATAL) << "No solution found.";
  }

  LOG(INFO) << "Total score = " << objective->Value() << "\n\n";


  //for (int s = 0; s < num_students; ++s) {
  //  if(MG4[s]->solution_value()) {
  //    cout << "MG4 is true for " << students[s] << "\n";
  //  }
  //}


  cout << "\nPROF VIEW\n";
  for (int p = 0; p < num_profs; ++p) {
    cout << profs[p]._name << "; ";
    for (int t = 0; t < num_times; ++t) {
      bool found_one= false;
      for (int s = 0; s < num_students; ++s) {
        if (Mpts[p][t][s]->solution_value() > 0.5) {
          found_one=true;
          cout << students[s]._name << ", ";
        }
      }
      if(!found_one) {
        cout << " ------- ";
      }
      cout << ";";
    }
    cout << "\n";
  }

  cout << "\nStudent VIEW\n";
  for (int s = 0; s < num_students; ++s) {
   cout << students[s]._name << "; ";

    for (int t = 0; t < num_times; ++t) {
      bool found_one= false;

      for (int p = 0; p < num_profs; ++p) {
        if (Mpts[p][t][s]->solution_value() > 0.5) {
          found_one=true;
          cout << profs[p]._name << ";";
        }
      }

      if(!found_one) {
        cout << " ------- ;";
      }
    }
    cout << "\n";
  }

  cout << "XXXXXXXXXXXXXXXXX PROFS XXXXXXXXXXXXXXXXX\n";

  int smallest_score=100000;
  int average_score=0;

  for (int p = 0; p < num_profs; ++p) {
    int score=0;
    int num_meetings=0;

    for (int t = 0; t < num_times; ++t) {
      for (int s = 0; s < num_students; ++s) {
        if(Mpts[p][t][s]->solution_value()>0.5) {
          score+=pref_score[make_pair(s,p)];        
          num_meetings+=1;
        }
      }
    }
    smallest_score=min(smallest_score,score);
    average_score+=score;



    cout << profs[p]._name << " " << num_meetings/(float)profs[p].slots() <<  " " << score << "\n";
  }
  cout << "Average Score: " << average_score/profs.size() << "\n";
  cout << "Smallest Score: " << smallest_score << "\n";



  cout << "XXXXXXXXXXXXXXXXX STUDENTS XXXXXXXXXXXXXXXXXXX\n";

  smallest_score=100000;
  average_score=0;
  for (int s = 0; s < num_students; ++s) {
    cout << students[s]._name << ": ";

    int score=0;
    for (int t = 0; t < num_times; ++t) {
      for (int p = 0; p < num_profs; ++p) {
        if(Mpts[p][t][s]->solution_value()>0.5) {
          score+=pref_score[make_pair(s,p)];        
        }
      }
    }
    smallest_score=min(smallest_score,score);
    average_score+=score;

    cout << score << "\n";
  }
  cout << "Average Score: " << average_score/students.size() << "\n";
  cout << "Smallest Score: " << smallest_score << "\n";


 // ----------------------------------------------------------------
 // EMAILS, CSV, PROFS

  ofstream ofs;
  ofs.open ("out-prof-email.csv",ios::trunc);

  for (int p = 0; p < num_profs; ++p) {
    ofs << profs[p]._name << ",";
    ofs << profs[p]._email << ",";
    //Start email
    ofs << "\"Thank you again for being involved in uArch 2021!  Your schedule (Timezone: EDT) for this Friday (18th) is below.  We've included some details about each student, just in case this information is helpful for context.\n\n";
    
    ofs << "Your Zoom Link:" << profs[p]._zoom_link << "\n\n";

    for (int t = 0; t < num_times; ++t) {
      if(t==0) ofs << "11:00am: \n";
      if(t==1) ofs << "11:20am: \n";
      if(t==2) ofs << "11:40am: \n";
      if(t==3) ofs << "2:00pm:  \n";
      if(t==4) ofs << "2:20pm:  \n";
      if(t==5) ofs << "2:40pm:  \n";

      bool found_one= false;
      for (int s = 0; s < num_students; ++s) {
        if (Mpts[p][t][s]->solution_value() > 0.5) {
          found_one=true;
          ofs << " * ";
          ofs << students[s]._name    << ", " <<
                  students[s]._email   << ", " <<
                  students[s]._univ    << ", " <<
                  students[s]._country << ", " <<
                  students[s]._degree  << ", " <<
                  "Grad Year: " << students[s]._grad_year
                  << "\n";
          ofs << "   Interests:";
          unsigned i = 0; 
          for(auto& t : students[s]._topics) {
            ofs << " " << t;
            i++;
            if(i!=students[s]._topics.size()) {
              ofs <<",";
            }
          }
          ofs << "\n\n";
        }
      }
      if(!found_one) {
        ofs << " ------- \n";
      }
    }
    ofs << "\nPlease let us know if anything comes up, and you can't make the meeting -- it should be no work around -- and of course let us know if you have any questions! \n\n Students will be coming from fairly high-level talks about architecture, research, and grad school life, so I imagine that all those topics will be on their minds. ";
    ofs << "\n\n";
    ofs << "Finally, we would like to list your name on the website to thank you, but please let us know if you would rather not have your name listed.\n\n";

    ofs << "Best Regards,\n";
    ofs << "Tony\n\n";
    ofs << "\"\n";
  }

  ofs.close();

  // ----------------------------------------------------------------
  // EMAILS, CSV, STUDENTS

  ofs.open ("out-student-email.csv",ios::trunc);

  for (int s = 0; s < num_students; ++s) {
    ofs << students[s]._name << ", ";
    ofs << students[s]._email << ", ";
    //ofs << "\"Hi " << students[s]._name << ",\n\n";

    ofs << "\"The following is your schedule for uArch this Friday (Timezone: EDT).  We recommend leaving a couple minutes between meetings to get to your next meeting on time.\n\n";

    for (int t = 0; t < num_times; ++t) {
      if(t==0) ofs << "11:00am: ";
      if(t==1) ofs << "11:20am: ";
      if(t==2) ofs << "11:40am: ";
      if(t==3) ofs << "2:00pm:  ";
      if(t==4) ofs << "2:20pm:  ";
      if(t==5) ofs << "2:40pm:  ";

      bool found_one= false;

      for (int p = 0; p < num_profs; ++p) {
        if (Mpts[p][t][s]->solution_value() > 0.5) {
          found_one=true;
          ofs << profs[p]._name << ",  Email: " << profs[p]._email << ", Meeting Link: " << profs[p]._zoom_link << "\n";
        }
      }

      if(!found_one) {
        ofs << " ---- BREAK ---- \n";
      }

    }

    ofs << "\nEvery prof will likely run their meeting a little differently, so just be prepared to introduce yourself, ask a question or too, and have fun!\n\n";

    ofs << "Best,\n";
    ofs << "uArch Team\n\n";

    ofs << "PS: the videos for the Applying to Grad School panel have now been posted on the uArch website. The links are next to each panelist's name.";

    ofs << "\"\n";
  }

  ofs.close();




}

#endif
