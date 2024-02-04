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

int findIndex(vector<int>& vec, int value)
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
//
//bool RogueCouple(int &uid, int &sid, vector<student> &slist, vector<university> &ulist)
//{
//    int uIndex = findIndex(ulist[uid].preference, sid); // university's preference of student
//    int sIndex = findIndex(slist[sid].preference, uid); // student's preference of university
//    
//    if (uIndex < uCurrent && sIndex < sCurrent)  // both prefer each other over current
//        return true;
//    else
//        return false;
//} 
// Generate a sequence of M random integers that sum to N
std::vector<int> 
generateVariables(int M, int N) {
    std::vector<int> variables;
    // Seed for the random number generator
    std::srand(std::time(0));
    // Generate M-1 random variables
    for (int i = 0; i < M - 1; ++i) {
        int randomValue = std::rand()%N;  // Generate a random value between 0 and N 
        if(randomValue == 0)   // Ensure that the random value is not 0
            randomValue = 1;
        variables.push_back(randomValue);
        N -= randomValue;     // Adjust N by subtracting the generated value
    }

    // The last variable is the remaining value needed to reach the target sum N
    variables.push_back(N);

    return variables;
}
const int GROUP_SIZE = 10;     // number of students in the group, 
const int UNIVERSITY_SIZE = 4; // number of universitys in the group
int main()
{
    vector<student> students;   
    vector<university> universitys;

    std::random_device rd, rd1;
    std::mt19937 gen(rd());
    std::mt19937 gen1(rd1());

    // Generate a sequence of integers for preference lists for students and universities
    std::vector<int> sequence;
    std::vector<int> sequence1;
    for (size_t i = 0; i < GROUP_SIZE; i++)
    {
        sequence.push_back(i);
    }
    for (size_t i = 0; i < UNIVERSITY_SIZE; i++)
    {
        sequence1.push_back(i);
    }

    // Generate a sequence of random capacities for universities
    vector<int> universityCapacity = generateVariables(UNIVERSITY_SIZE,GROUP_SIZE);

    // Create students with random university preferences
    for (int i = GROUP_SIZE - 1; i >= 0; i--)
    {
        std::mt19937 gen1(rd1());
        // Shuffle the sequence using the Fisher-Yates shuffle algorithm
        std::shuffle(sequence1.begin(), sequence1.end(), gen1);
        students.push_back(student(GROUP_SIZE - 1 - i, sequence1));
    }
    // Create universitys with random student preferences
    for (int i = 0; i < UNIVERSITY_SIZE; i++)
    {
        std::mt19937 gen(rd());
        std::shuffle(sequence.begin(), sequence.end(), gen);
        universitys.push_back(university(UNIVERSITY_SIZE - 1 - i, sequence));
    }
   
    auto start_time = std::chrono::high_resolution_clock::now(); // start time
 
    // Match using modified Gale-Shapley algorithm with capacity constraints
    vector<vector<int>> universityLists;
    vector<int> universityList;
    // University lists initialized with empty lists, list of students visiting university i
    for (int i = 0; i < UNIVERSITY_SIZE; i++)
        universityLists.push_back(universityList);

    // Start matching
    for (;;)
    {
        // students go to visit top university on their list
        for (vector<student>::iterator studentIter = students.begin(); studentIter != students.end(); studentIter++)
        {
            int universityId = (*studentIter).preference[0];  // highest preference

            auto it = std::find(universityLists[universityId].begin(), universityLists[universityId].end(), (*studentIter).id);

            // Check if student already in visit list, if not add to list
            if (it == universityLists[universityId].end())
                universityLists[universityId].push_back((*studentIter).id);   
        }

        // Check for termination, each university reaches exact capacity
        int suitors = 0;
        for (int iter = 0;iter < UNIVERSITY_SIZE; iter++)
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
            for (int i = 0; i < UNIVERSITY_SIZE; i++)
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
    for (size_t i=0; i< universityLists.size(); i++)
    {
        for (size_t j=0;j<universityLists[i].size();j++)
            students[universityLists[i][j]].engaged = i; 
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed_seconds = end_time - start_time;
     std::cout << "University Capacity" << '\n';
     for (std::size_t i = 0; i < universityCapacity.size(); ++i) {
         std::cout << "University " << i << ": " << universityCapacity[i] << '\n';
     }
     std::cout << "Students attending universities" << '\n';
     vector<vector<int>>::iterator itr =  universityLists.begin();
     for (;itr!=universityLists.end();itr++)
     {
         for (auto x : *itr)
         {
             std::cout << " Student " << x << " will attend University: " << distance(universityLists.begin(),itr)  << '\n';
         }
     }
    std::cout << "Elapsed time: " << elapsed_seconds.count() << " seconds." << std::endl;
    return 0;
}
