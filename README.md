# Genetic-Improvement
I was a part of the SIGEVO Summer School (S3), the first summer school organized by SIGEVO, ACM's chapter which deals with evolutionary computing. It was a great experience wherein we worked on different assignments floated by different researchers in this area.
Link: http://gecco-2017.sigevo.org/index.html/Summer+School

Among many interesting assignment topics, Genetic Improvement ([by Petke et al.](http://www.cs.virginia.edu/~weimer/p/weimer-tse2017.pdf) from UCL's [GISMO lab](http://www0.cs.ucl.ac.uk/staff/ucacbbl/gismo/)) was of natural curiosity to me because it involved modifying/synthesizing source code to attain a specified functionality. The primary author, Dr. Justyna Petke, was one of the lecturers at the summer school and was my supervisor for this project (since she had floated the project idea). 

We have seen this work before in the form of program synthesis (Gulwani et al.) and program repair (Rinard et al., Le Gouse et al.) and the question to naturally ask and answer was - how GI was really different? What did it bring to the table which the other techniques did not? Tradeoffs with other techniques, etc.

My assignment involved installing the GI framework on my local machine, installing a generic SAT-solver and then using GI to modify its source code to synthesize a solver for the Combinatorial Interaction Testing problem, a domain-specific application of the generic sAT-problem.

This repository includes
* The source code which was given to us, which includes some shell scripts (for the GI framework) and the code for the generic SAT-solver (miniSAT).
* A log of the different changes I made to the code to get it running
* Notes and FAQs on different aspects of the framework, and some critical questions to help understand the nuances. This was largely from the conversations and correspondences I had with Justyna at the summer school.
* Helper scripts I wrote (written in Python) to assimilate results from the logs that get generated.
* Results from the runs I ran, along with some notes and observations.
* Note: I have a separate repository wherein I've pushed the material we received and my responses to the assignments and challenges I wrote out. I will link some of that material with my notes here. They overlapped.

In case of any queries, feel free to write to me at shash at-sign mit dot-symbol edu
