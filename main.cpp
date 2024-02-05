#include <iostream>
#include <chrono>
#include <vector>
#include <random>
using namespace std;
struct university
{
    int id;
    std::vector<int> preference;  // preference list ordered with most preferred first
    int engaged;
    university(int id, std::vector<int> preference) : id(id), preference(preference), engaged(-1) {}
};

struct student
{
    int id;
    std::vector<int> preference;   // preference list ordered with most preferred first
    int engaged;
    student(int id, std::vector<int> preference) : id(id), preference(preference), engaged(-1) {}
};

int findIndex(const vector<int> &vec, const int &value)
{
    for (int i = 0; i < vec.size(); i++)
    {
        if (vec[i] == value)
        {
            return i;
        }
    }
    return -1;
}

//Predicate used for sorting of students at university i by university preference
bool sortStudent(int dat1, int dat2, vector<university> &uni, int unidex)
{   int index1 = findIndex(uni[unidex].preference, dat1);
    int index2 = findIndex(uni[unidex].preference, dat2);
    return (index1 <  index2);
}
// Rougue Couple check - 
bool RogueCouple(int &s,const vector<student> &slist, const vector<university> &ulist)
{
    int mIndex = findIndex(slist[s].preference, slist[s].engaged);  // spouse index in preference list
    if (mIndex != 0)
    {
        for (int i = mIndex; i >= 0; --i)
        {
            int uIndex = findIndex(ulist[slist[s].preference[i]].preference, s);  // current student index in preference list
            int uCurrent = findIndex(ulist[slist[s].preference[i]].preference, slist[s].engaged);  // current student index in preference list
            if (uIndex < uCurrent)
                return true;
        }
    }
    return false;
}
// Generate a sequence of M random integers that sum to N
std::vector<int> 
generateVariables(const int &M,const int &N) {
    std::vector<int> variables;
    // Seed for the random number generator
    std::srand(std::time(0));
    // Generate M-1 random variables
    int summ = 0;
    int NN = N;   // nonconst copy of N
    for (int i = 0; i < M - 1; ++i) {
        int randomValue = std::rand()%NN/2;  // Generate a random value between 0 and N/2 
        variables.push_back(randomValue);
        if(NN - randomValue > 0 )
             NN -= randomValue;     // Adjust N by subtracting the generated value
        summ += randomValue;
    }
    // The last variable is the remaining value needed to reach the target sum N
    variables.push_back(N-summ);
    return variables;
}
const int GROUP_SIZE = 400;     // number of students in the group, 
const int UNIVERSITY_SIZE = 10; // number of universitys in the group
int main()
{
    vector<student> students;   
    vector<university> universitys;

    std::random_device rd, rd1;
    std::mt19937 gen(rd());
    std::mt19937 gen1(rd1());

    // Generate a sequence of integers for preference lists for students 
    // and universities
    std::vector<int> sequence;
    std::vector<int> sequence1;
    for (size_t i = 0; i < GROUP_SIZE; ++i)
    {
        sequence.push_back(i);
    }
    for (size_t i = 0; i < UNIVERSITY_SIZE; ++i)
    {
        sequence1.push_back(i);
    }

    // Generate a sequence of random capacities for universities
    vector<int> universityCapacity = generateVariables(UNIVERSITY_SIZE,GROUP_SIZE);

    // Create students with random university preferences
    for (int i = GROUP_SIZE - 1; i >= 0; --i)
    {
        std::mt19937 gen1(rd1());
        // Shuffle the sequence using the Fisher-Yates shuffle algorithm
        std::shuffle(sequence1.begin(), sequence1.end(), gen1);
        students.push_back(student(GROUP_SIZE - 1 - i, sequence1));
    }
    // Create universitys with random student preferences
    for (int i = 0; i < UNIVERSITY_SIZE; ++i)
    {
        std::mt19937 gen(rd());
        std::shuffle(sequence.begin(), sequence.end(), gen);
        universitys.push_back(university(UNIVERSITY_SIZE - 1 - i, sequence));
    }
   
    auto start_time = std::chrono::high_resolution_clock::now(); // start time
 
    // Match using modified Gale-Shapley algorithm with capacity constraints
    std::vector<int>* dynamicVector = new std::vector<int>();

    vector<vector<int>> universityLists;
    vector<int> universityList;
   
    // University lists initialized with empty lists, list of students visiting university i
    for (int i = 0; i < UNIVERSITY_SIZE; ++i)
        universityLists.push_back(universityList);
    // Start matching
    for (;;)
    {
        // students go to visit top university on their list
        vector<student>::iterator end1 = students.end();
        for (vector<student>::iterator studentIter = students.begin(); studentIter != end1; ++studentIter)
        {
            int universityId = (*studentIter).preference[0];  // highest preference

            auto it = std::find(universityLists[universityId].begin(), universityLists[universityId].end(), (*studentIter).id);

            // Check if student already in visit list, if not add to list
            if (it == universityLists[universityId].end())
                universityLists[universityId].push_back((*studentIter).id);   
        }
        // Check for termination, each university reaches exact capacity
        int suitors = 0;
        for (int iter = 0;iter < UNIVERSITY_SIZE; ++iter )
        {
            if (universityLists[iter].size() != universityCapacity[iter])  // if university has not reached capacity
                break;
            else
                suitors++;
        }
        if (suitors == UNIVERSITY_SIZE)   // Done with matching
            break;
        // continue matching, universities accept top ranked student visiting upto capacity and reject others
        else    
        {
            for (int i = 0; i < UNIVERSITY_SIZE; ++i)
            {   // group exceeds capacity, sort list and reject overflow
                if (universityLists[i].size() > universityCapacity[i])  
                {
                    std::sort(universityLists[i].begin(), universityLists[i].end(), [&](int id1, int id2) {
                        return sortStudent(id1, id2, universitys, i);
                        });
                   //  students remove university from list if rejected, top of their preference list gets popped
                    for (int j = universityLists[i].size() - 1;j >= universityCapacity[i]; --j)
                    {
                      students[universityLists[i][j]].preference.erase(students[universityLists[i][j]].preference.begin());
                      universityLists[i].pop_back();  // remove student from visit list
                    }
                }
            }
        }
    }
    // Update engaged status for students
    for (size_t i=0; i< universityLists.size(); ++i )
    {
        for (size_t j=0;j<universityLists[i].size();++j)
            students[universityLists[i][j]].engaged = i; 
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed_seconds = end_time - start_time;
    // Output results
     std::cout << "University Capacity" << '\n';
     for (std::size_t i = 0; i < universityCapacity.size(); ++i) {
         std::cout << "University " << i << ": " << universityCapacity[i] << '\n';
     }
    /* std::cout << "Students attending universities" << '\n';
     vector<vector<int>>::iterator endd = universityLists.end();
     vector<vector<int>>::iterator itr =  universityLists.begin();
     for (;itr!=endd;++itr)
     {
         for (auto x : *itr)
         {
             std::cout << " Student " << x << " will attend University: " 
                 << distance(universityLists.begin(), itr) << " is rogue "
                 << (bool)RogueCouple(x, students, universitys) << '\n';
         }
     }*/
    std::cout << "Elapsed time: " << elapsed_seconds.count() << " seconds." << std::endl;
    
    // Forced Rogue
    vector<student> StudentsR;
    vector<university> UniversitysR;
    student s0(0, { 0,1 });
    student s1(1, { 1,0 });
    university u0(0, { 0,1 });
    university u1(1, { 1,0 });
    u0.engaged = 1;            // force university 0 to be engaged to student 1
    u1.engaged = 0;            // force university 1 to be engaged to student 0
    s0.engaged = 1;            // force student 0 to be engaged to university 1
    s1.engaged = 0;            // force student 1 to be engaged to university 0
    StudentsR.push_back(s0);
    StudentsR.push_back(s1);
    UniversitysR.push_back(u0);
    UniversitysR.push_back(u1);
    cout << "Forced Rouge Couple" << " Student: " << 1 << " University: " << 0 << " is rouge : "
        << (bool)RogueCouple(u1.id, StudentsR, UniversitysR) << '\n';
    StudentsR.clear();
    UniversitysR.clear();
    u0.engaged = 0;            // force university 0 to be engaged to student 0
    u1.engaged = 1;            // force university 1 to be engaged to student 1
    s0.engaged = 0;            // force student 0 to be engaged to university 0
    s1.engaged = 1;            // force student 1 to be engaged to university 1
    StudentsR.push_back(s0);
    StudentsR.push_back(s1);
    UniversitysR.push_back(u0);
    UniversitysR.push_back(u1);
    cout << "Rouge Couple test on stable matching " << " Student: " << 1 << " University: " << 0 << " is rouge : "
        << (bool)RogueCouple(u1.id, StudentsR, UniversitysR) << '\n';
    return 0;
}
