#include <iostream>
#include <stdio.h>
#include <queue>
#include <cstdlib>
#include <unistd.h>
#include <fcntl.h>
#include <limits>
#include <string.h>
#include <math.h>

using namespace std;

typedef struct
{
    char processName;
    int arrivalTime;
    int serviceTime;
    int remainingTime;
    int finishTime;
    int turnAroundTime;
    float normTurnAroundTime;
    int initialPriority;
    int priority;
    int timeInFront;
    char* trace;
} Process;

Process* Process_init(char processName, int arrivalTime, int serviceTime, int totalTime)
{
    Process* p = (Process*)malloc(sizeof(Process));
    p->processName = processName;
    p->arrivalTime = arrivalTime;
    p->serviceTime = serviceTime;
    p->remainingTime = serviceTime;
    p->finishTime = 0;
    p->turnAroundTime = 0;
    p->normTurnAroundTime = 0;
    p->timeInFront = 0;
    p->initialPriority = serviceTime;
    p->priority = serviceTime;
    p->trace = (char*)malloc(sizeof(char)*(totalTime+1));
    for (int i = 0 ; i < totalTime ; i++)
    {
        p->trace[i] = ' ';
    }
    p->trace[totalTime] = '\0';
    return p;
}

void insertWaiting(Process* p)
{
    for (int i = p->arrivalTime ; i <= p->finishTime-1 ; i++)
    {
        if (p->trace[i] != '*')
        {
            p->trace[i] = '.';
        }
    }
}

void calculateTimes(Process* p)
{
    p->turnAroundTime = p->finishTime - p->arrivalTime;
    p->normTurnAroundTime = (float)p->turnAroundTime / (float)p->serviceTime;
    insertWaiting(p);
}

void executeProcess(Process* p, int t)
{
    p->timeInFront++;
    p->remainingTime--;
    p->trace[t] = '*';
}

int getWaitingTime(Process* p, int t)
{
    return t - p->arrivalTime - p->serviceTime + p->remainingTime;
}

int getExecutedTime(Process* p)
{
    return p->serviceTime - p->remainingTime;
}

int getNextLevel(int i)
{
    if (i == 4)
    {
        return i;
    }
    return i+1;
}

int getSum(int n)
{
    if (n == 1 | n == 0)
    {
        return n;
    }
    return n + getSum(n-1);
}

void centrePrint(int n, int width)
{
    char s[20] = {'\0'};
    int len;
    sprintf(s, "%d", n);
    len = strlen(s);
    if (len >= width)
        printf(s);
    else
    {
        int remaining = width - len;
        int spacesRight = (remaining / 2 );
        int spacesLeft = remaining - spacesRight-1;
        printf("%*s%s%*s", spacesLeft, "", s, spacesRight, "");
    }
}

void printTrace(Process* arr[], int n, int time)
{
    for (int i = 0 ; i < time+1 ; i++)
    {
        printf("%d ", i%10);
    }
    printf("\n");
    printf("------------------------------------------------\n");
    for (int i = 0 ; i < n ; i++)
    {
        printf("%-6c", arr[i]->processName);
        for (int t = 0 ; t<time ; t++)
        {
            printf("|%c", arr[i]->trace[t]);
        }
        printf("| ");
        printf("\n");
    }
    printf("------------------------------------------------\n\n");
    return;
}

void printStats(Process* arr[], int n, int time)
{
    char title[20];

    sprintf(title, "Process");
    printf("%-11s",title);
    for (int i = 0; i<n ; i++)
    {
        printf("|  %c  ", arr[i]->processName);
    }
    printf("|\n");

    sprintf(title, "Arrival");
    printf("%-11s",title);
    for (int i = 0; i<n ; i++)
    {
        printf("|");
        centrePrint(arr[i]->arrivalTime,6);
    }
    printf("|\n");

    sprintf(title, "Service");
    printf("%-11s",title);
    for (int i = 0; i<n ; i++)
    {
        printf("|");
        centrePrint(arr[i]->serviceTime,6);
    }
    printf("| Mean|\n");

    sprintf(title, "Finish");
    printf("%-11s",title);
    for (int i = 0; i<n ; i++)
    {
        printf("|");
        centrePrint(arr[i]->finishTime,6);
    }
    printf("|-----|\n");

    float mean = 0;

    sprintf(title, "Turnaround");
    printf("%-11s",title);
    for (int i = 0; i<n ; i++)
    {
        mean += arr[i]->turnAroundTime;
        printf("|");
        centrePrint(arr[i]->turnAroundTime,6);
    }
    mean = mean / n;
    printf("|%5.02f|\n", mean);

    mean = 0;

    sprintf(title, "NormTurn");
    printf("%-11s",title);
    for (int i = 0; i<n ; i++)
    {
        mean += arr[i]->normTurnAroundTime;
        printf("|%5.02f", arr[i]->normTurnAroundTime);
    }
    mean = mean / n;
    printf("|%5.02f|\n\n", mean);

    return;
}

void printProcess(Process* p)
{
    printf("\t%c\t%d\t%d\t%d\t%d\t%.2f \n", p->processName, p->arrivalTime, p->serviceTime, p->finishTime, p->turnAroundTime, p->normTurnAroundTime);
}

void firstComeFirstServe(Process* arr[], int n, int time)
{
    queue<Process*> q;
    for(int t = 0 ; t <= time ; t++ )
    {
        for(int i = 0 ; i < n ; i++)
        {
            if (arr[i]->arrivalTime == t)
            {
                q.push(arr[i]);
            }
        }
        if (q.front()->remainingTime == 0)
        {
            q.front()->finishTime = t;
            calculateTimes(q.front());
            q.pop();
        }
        if (!q.empty())
        {
            executeProcess(q.front(), t);
        }
    }
}

void roundRobin(Process* arr[], int n, int time, int timeSlot)
{
    queue<Process*> q;
    int quantum = 0;
    for(int t = 0 ; t <= time ; t++ )
    {
        for(int i = 0 ; i < n ; i++)
        {
            if (arr[i]->arrivalTime == t)
            {
                q.push(arr[i]);
            }
        }
        if (!q.empty())
        {
            if (quantum == timeSlot)
            {
                q.push(q.front());
                q.pop();
                quantum = 0;
            }
            if(q.front()->remainingTime != 0)
            {
                executeProcess(q.front(),t);
                quantum++;
                if(q.front()->remainingTime == 0)
                {
                    q.front()->finishTime = t+1;
                    calculateTimes(q.front());
                    q.pop();
                    quantum = 0;
                }
            }
        }
    }
}

void shortestProcessNext(Process* arr[], int n, int time)
{
    Process* activeProcess;
    int active = 0;
    int minimum = 100000;
    for(int t = 0 ; t <= time ; t++ )
    {
        if (!active)
        {
            for(int i = 0 ; i < n ; i++)
            {
                if (arr[i]->arrivalTime <= t && arr[i]->remainingTime > 0)
                {
                    if (arr[i]->serviceTime < minimum)
                    {
                        minimum = arr[i]->serviceTime;
                        activeProcess = arr[i];
                        active = 1;
                    }
                }
            }
        }
        if (active)
        {
            if (activeProcess->remainingTime != 0)
            {
                executeProcess(activeProcess,t);
                if (activeProcess->remainingTime == 0)
                {
                    activeProcess->finishTime = t+1;
                    calculateTimes(activeProcess);
                    active = 0;
                    minimum = 100000;
                }
            }
        }
    }
}

void shortestRemainingTime(Process* arr[], int n, int time)
{
    Process* activeProcess;
    int active = 0;
    int minimum = 100000;
    for(int t = 0 ; t <= time ; t++ )
    {
        for(int i = 0 ; i < n ; i++)
        {
            if (arr[i]->arrivalTime <= t && arr[i]->remainingTime > 0)
            {
                if (arr[i]->remainingTime < minimum)
                {
                    minimum = arr[i]->remainingTime;
                    activeProcess = arr[i];
                    active = 1;
                }
            }
        }
        if (active)
        {
            if (activeProcess->remainingTime != 0)
            {
                executeProcess(activeProcess,t);
                if (activeProcess->remainingTime == 0)
                {
                    activeProcess->finishTime = t+1;
                    calculateTimes(activeProcess);
                    active = 0;
                    minimum = 100000;
                }
            }
        }
    }
}

void highestResponseRatioNext(Process* arr[], int n, int time)
{
    Process* activeProcess;
    int active = 0;
    int maximum = -1;
    for(int t = 0 ; t <= time ; t++ )
    {
        if (!active)
        {
            for(int i = 0 ; i < n ; i++)
            {
                if (arr[i]->arrivalTime <= t && arr[i]->remainingTime > 0)
                {
                    float responseRatio = (float)(getWaitingTime(arr[i], t) + arr[i]->serviceTime) / (float)arr[i]->serviceTime;
                    if (responseRatio > maximum)
                    {
                        maximum = responseRatio;
                        activeProcess = arr[i];
                        active = 1;
                    }
                }
            }
        }
        if (active)
        {
            if (activeProcess->remainingTime != 0)
            {
                executeProcess(activeProcess,t);
                if (activeProcess->remainingTime == 0)
                {
                    activeProcess->finishTime = t+1;
                    calculateTimes(activeProcess);
                    active = 0;
                    maximum = 0;
                }
            }
        }
    }
}

void feedback(Process* arr[], int n, int time)
{
    queue<Process*> q[5];
    int num = 0;
    for(int t = 0 ; t <= time ; t++ )
    {
        for(int i = 0 ; i < n ; i++)
        {
            if (arr[i]->arrivalTime == t)
            {
                q[0].push(arr[i]);
                num++;
            }
        }

        for(int i=0 ; i < 5 ; i++)
        {
            if (!q[i].empty())
            {
                if (q[i].front()->remainingTime !=0)
                {
                    if (num != 1)
                    {
                        if ( q[i].front()->timeInFront - 1 >= 0 )
                        {
                            q[getNextLevel(i)].push(q[i].front());
                            q[i].front()->timeInFront = 0;
                            q[i].pop();
                        }
                    }
                    if (!q[i].empty())
                    {
                        executeProcess(q[i].front(), t);
                        if (q[i].front()->remainingTime == 0)
                        {
                            q[i].front()->finishTime = t+1;
                            calculateTimes(q[i].front());
                            q[i].pop();
                            num--;
                        }
                        break;
                    }
                }
            }
        }
    }

}

void feedback2(Process* arr[], int n, int time)
{
    queue<Process*> q[5];
    int num = 0;
    int active = 0;
    for(int t = 0 ; t <= time ; t++ )
    {
        for(int i = 0 ; i < n ; i++)
        {
            if (arr[i]->arrivalTime == t)
            {
                q[0].push(arr[i]);
                num++;
            }
        }

        for(int i=active ; i < 5 ; i++)
        {
            if (!q[i].empty())
            {
                if (q[i].front()->remainingTime !=0)
                {
                    if (num != 1)
                    {
                        if ( q[i].front()->timeInFront - pow(2,i) >= 0 )
                        {
                            q[getNextLevel(i)].push(q[i].front());
                            q[i].front()->timeInFront = 0;
                            q[i].pop();
                            i = active = 0;
                        }
                    }
                    if (!q[i].empty())
                    {
                        executeProcess(q[i].front(), t);
                        active = i;
                        if (q[i].front()->remainingTime == 0)
                        {
                            q[i].front()->finishTime = t+1;
                            calculateTimes(q[i].front());
                            q[i].pop();
                            num--;
                            active = 0;
                        }
                        break;
                    }
                }
            }
        }
    }

}

void aging(Process* arr[], int n, int time, int timeSlot)
{
    queue<Process*> q;
    Process* activeProcess = NULL;
    Process* frontOfQ;
    int active = 0;
    int maximum = -1;

    for (int t = 0; t < time ; t++)
    {
        for (int i = 0 ; i < n ; i++)
        {
            if (arr[i]->arrivalTime == t)
            {
                q.push(arr[i]);
            }
        }
        if (!active)
        {
            if (!q.empty())
            {
                // incrementing priorities of processes in ready queue
                frontOfQ = q.front();
                q.front()->priority++;
                q.push(frontOfQ);
                q.pop();
                while (q.front() != frontOfQ)
                {
                    q.front()->priority++;
                    q.push(q.front());
                    q.pop();
                }
            }

            if (activeProcess != NULL)
            {
                q.push(activeProcess);
            }

            active = 1;
            // get maximum
            frontOfQ = q.front();
            maximum = q.front()->priority;
            activeProcess = q.front();
            q.push(q.front());
            q.pop();

            while (frontOfQ != q.front())
            {
                if (maximum < q.front()->priority)
                {
                    maximum = q.front()->priority;
                    activeProcess = q.front();
                }
                q.push(q.front());
                q.pop();
            }

            // poping maximum
            frontOfQ = q.front();
            if (frontOfQ == activeProcess)
            {
                q.pop();
                activeProcess->timeInFront = 0;
                activeProcess->priority = activeProcess->initialPriority;
            }
            else
            {
                q.push(q.front());
                q.pop();
                while ( frontOfQ != q.front() )
                {
                    if ( q.front() == activeProcess)
                    {
                        q.pop();
                        activeProcess->timeInFront = 0;
                        //activeProcess->priority = activeProcess->initialPriority;
                    }
                    else
                    {
                        q.push(q.front());
                        q.pop();
                    }
                }
            }
        }
        // Execution
        if (active)
        {
            executeProcess(activeProcess, t);
            if (activeProcess->timeInFront == timeSlot)
            {
                activeProcess->priority = activeProcess->initialPriority;
                maximum = -1;
                active = 0;
            }
        }
    }


    for (int i = 0 ; i < n ; i++)
    {
        for (int t = arr[i]->arrivalTime ; t < time ; t++)
        {
            if (arr[i]->trace[t] != '*')
            {
                arr[i]->trace[t] = '.';
            }
        }
    }
}

int main()
{
    char mode[10];
    int algo;
    int q;
    int time;
    int n;
    scanf("%s", mode);
    scanf("%d-%d",&algo,&q);
    scanf("%d",&time);
    scanf("%d",&n);
    Process* processList[n];
    char name;
    int arrival;
    int service;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    for (int i = 0 ; i < n ; i++)
    {
        scanf("%c,%d,%d",&name,&arrival,&service);
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        processList[i] = Process_init(name,arrival,service,time);
    }

    char title[10];
    switch(algo)
    {
    case 1: // First Come First Serve
        firstComeFirstServe(processList, n, time);
        sprintf(title,"FCFS");
        break;

    case 2: // Round Robin with quantum = q
        roundRobin(processList, n, time, q);
        sprintf(title,"RR-%d",q);
        break;

    case 3: // Shortest Process Next
        shortestProcessNext(processList, n, time);
        sprintf(title,"SPN");
        break;

    case 4: // Shortest Remaining Time
        shortestRemainingTime(processList, n, time);
        sprintf(title,"SRT");
        break;

    case 5: // Highest Response Ratio Next
        highestResponseRatioNext(processList, n, time);
        sprintf(title,"HRRN");
        break;

    case 6: // Feedback-1
        feedback(processList, n, time);
        sprintf(title,"FB-1");
        break;

    case 7: // Feedback-2i
        feedback2(processList, n, time);
        sprintf(title,"FB-2i");
        break;

    case 8: // Aging
        aging(processList, n, time, q);
        sprintf(title,"Aging");
        break;

    default: // rejected input
        return 0;
    }

    if (!strcmp(mode, "trace"))
    {
        printf("%-6s",title);
        printTrace(processList, n, time);
    }
    else if (!strcmp(mode, "stats"))
    {
        printf("%s\n",title);
        printStats(processList, n, time);
    }

    return 0;
}
