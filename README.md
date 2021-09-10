# uarch-scheduling

This is the scheduling tool for the UArch mentoring workshop, used in 2021.

### Dependences

The only dependence is google optimization tools:

https://developers.google.com/optimization/install

### Inputs/Outputs

The inputs are in two TSV files: prof.tsv, and students.tsv. TSV was used to allow fields to contain commas, without increasing the complexity of parsing.

The mapping will be printed on stdout.  The outputs are in corresponding csv files, and contain the email message to students and professors.

### Make and Run

To run:

`> make`

`> build/assigner`

### Notes

Many aspects of the input and scheduling are hard-coded, including the number of slots.

Note that for the parsing to work, the names of professors and names of topics must match exactly between the students.tsv and profs.tsv.  It is recommended to supply a drop-down selector on the data collection forms (e.g. with google forms) with professor names and topics so this will be gauranteed by defult.

Many aspects of the scheduling can be changed by tweaking the rules in src/assignment.h


