#include <bits/stdc++.h>
#include <fstream>
using std::ifstream;

using namespace std;

#define MAX_USER_COUNT 100 // maximum number of users allowed 
#define CORE_COUNT 4 // number of cores in system
#define CONTEXT_SWITCH_TIME 0.1 // context switch time
#define MAX_REQUEST_GENERATED 20 // 
#define MAX_THREAD_COUNT 4  // Number of cores per thread
#define MAX_BUFFER_SIZE 1000 // Server Buffer storage for incoming messages when no core is free
#define TIME_QUANTUM 0.5 // Defined for Round Robin


enum serverStatus {Idle = 1, Busy=2}; // 2 states of server
enum eventType {arrival=1,departure=2,contextSwitchInEvent=3,contextSwitchOutEvent=4}; //types of event dealt with in the simulation
enum schedulingPolicy{FCFS=1,roundRobin=2}; //scheduling policies the system supports
enum Distributions { Exponential=1, Uniform, Constant }; // types pof distribution system supports
enum bufferStatus {Full=1,Available=2,Empty=3}; // buffer status representation

ofstream outdata; // used to write final output data to file "outfile.csv"
ofstream outTrace; // used to write trace of the system to file "Trace.txt"
double meanWaitingTime = 0.0;
double meanResponseTime = 0.0;
unsigned int request_drops = 0;

/**
 * Class Service Time
 * @brief Generates Random service time.
 *  attributes  :   Dsitribution ds (enum)
 *              :   mean_service (double)
 * 
 *  Methods     :  double getServiceTime() 
 *     
 */
class Service_Time {
    public:

    Distributions ds;
    double mean_service;

    /**
     * @brief Construct a new Service_Time object
     * 
     */
    Service_Time(){
        ds = Uniform;
        mean_service = 5;
    }
    /**
     * @brief Construct a new Service_Time object
     * 
     * @param user_mean_service 
     * @param user_ds 
     */
    Service_Time(int user_mean_service, Distributions user_ds){
      this->mean_service  = user_mean_service;
      this->ds = user_ds;  
    }

    /**
     * @brief Get the Service Time object
     * 
     * @return double 
     */
    double getServiceTime() {
        double final_service_time = 0.0;
        std::random_device device;     
        std::mt19937 generator(device());   
        std::uniform_int_distribution<int> dist(2,2*mean_service+2); 
        std::uniform_real_distribution<float> dist1(0, 1);

        switch(ds) {
            case Exponential:
                final_service_time = -mean_service * log(dist1(generator));
                break;
            
            case Uniform:
                final_service_time = dist(generator);
                break;

            case Constant:
                final_service_time = mean_service;
                break;
        }
        return final_service_time;
    }  
};
/** Class Timeout
 * @brief generates Random timeout time required by events
 * Attributes   :   Distribution ds (enum)
 *              :   mean_time (double)
 *              :   minimum_timeout(int)
 * 
 * Methods      :   double getTimeout()      
 */
class Timeout {
    public:

    Distributions ds;
    double mean_time;
    int minimum_timeout = 100;

    /**
     * @brief Construct a new Timeout object
     * 
     */
    Timeout(){
        ds = Uniform;
        mean_time = 3;
    }

    /**
     * @brief Construct a new Timeout object
     * 
     * @param user_mean_time 
     * @param user_ds 
     */
    Timeout(int user_mean_time, Distributions user_ds){
      this->mean_time  = user_mean_time;
      this->ds = user_ds;  
    }

    /**
     * @brief Get the Timeout object
     * 
     * @return double 
     */
    double getTimeout() {
        double final_timeout = 0.0;
        std::random_device device;     
        std::mt19937 generator(device());   
        std::uniform_int_distribution<int> dist(1,2*mean_time+1);
        std::uniform_real_distribution<float> dist1(0, 1);

        switch(ds) {
            case Exponential:
                final_timeout = (-mean_time * log(dist1(generator))) + minimum_timeout;
                break;
            
            case Uniform:
                final_timeout = dist(generator)+minimum_timeout;
                break;

            case Constant:
                final_timeout = mean_time + minimum_timeout;
                break;
        }
        return final_timeout;
    }  
};


/**
 * @brief used to store User Input values  
 * 
 * Attributes   :   meanTimeoutTime (double)
 *              :   meanServiceTime (double)
 *              :   noOfUsers (unsigned int)
 *              :   naxRequestPerUser(unsigned int)
 *              :   serviceTimeDistribution       (Distributions) 
 *              :   timeoutTimeDistribution       (Distributions) 
 *              :   schedulingPolicy         (policy)
 */ 
class UserData{
    public:
        double meanTimeoutTime;
        double meanServiceTime;
        unsigned int noOfUsers;    
        unsigned int maxRequestPerUser;
        Distributions serviceTimeDistribution;
        Distributions timeotTimeDistribution;
        schedulingPolicy policy;
};
/**
 * @brief the request instance unique to each user 
 * Attributes       :         (int) objectId
 *                  :         (int) eventId
 *                  :         (eventType) type
 *                  :         (double) arrivalTime
 *                  :         (double) timeout
 *                  :         (double) contextSwitchInTime
 *                  :         (double) contextSwitchOutTime
 *                  :         (double) serviceTime
 *                  :         (double) departureTime
 *                  :         (double) waitingTime
 *                  :         (int) core
 *                  :         (int) thread
 *                  :         (int) response_count
 *                  :         (int) request_count
 *                  :         static (int)  object_count
 * 
 * Methods          :       double getRandomThinkTime()
 *                  :       double getRemainingServiceTime()
 */
class Event {
    public:
       int objectId;
       int eventId;
       eventType type;
       double arrivalTime;
       double timeout;
       double contextSwitchInTime;
       double contextSwitchOutTime;
       double serviceTime;
       double eventServiceTime;
       double departureTime;
       double waitingTime;
       int core;
       int thread;
       int response_count;
       int request_count;
       static int  object_count;

       /**
        * @brief Gets a Random Think Time 
        * 
        * @return * double 
        */
       double getRandomThinkTime();
       /**
        * @brief Get the Remaining Service Time of the request
        * 
        * @return double 
        */
       double getRemainingServiceTime(); 
};

double Event::getRandomThinkTime(){
    std::random_device device;     
    std::mt19937 generator(device());    
    std::uniform_int_distribution<int> dist(4,10);
    auto thinkTime = dist(generator);
    return (double)thinkTime;
}

double Event::getRemainingServiceTime(){
    return serviceTime;
}

/**
 * @brief Used to schedule the requests on the core
 * 
 * Attributes   :   type (schedulingPolicy)
 *              :   contextSwitchTime (int)
 */
class Scheduler{
    public:
        
        schedulingPolicy type;
        int contextSwitchTIme;

        Scheduler(){}
        Scheduler(schedulingPolicy p);
        void setPolicy(schedulingPolicy p);
};

/**
 * @brief Construct a new Scheduler:: Scheduler object
 * 
 * @param p 
 */
Scheduler::Scheduler(schedulingPolicy p){
    type=p;
    contextSwitchTIme = CONTEXT_SWITCH_TIME;
}

/**
 * @brief Set the Policy
 * 
 * @param p 
 */
void Scheduler::setPolicy(schedulingPolicy p){
    this->type = p;
}

/**
 * @brief Each represents a core of the system
 * Attributes : threads (queue of Event Class instance Pointers)
 *            : status (serverStatus)  
 *            : threadBusyCount (int)
 *            : schedulerObj (Scheduler Class Instance)
 *            : coreId (int identification of the core)
 *            : coreIdIterator (static Int to assign new Id to each new instance of Core Class)        
 * 
 * 
 * Methods      :  double getNextCoreAvailableTime()
 *              :  serverStatus getCoreStatus()
 *              :  void setCoreStatus(serverStatus status)
 *              :  int addToThread(Event obj)
 *              :  Event* removeFromThread()
 *              :  int getBUsyThreadCount()
 *              :  void setBusyThreadCount(int count)
 */
class Core{
    public:
    
        std::queue<Event*> threads;
        serverStatus status;
        int threadBusyCount;
        Scheduler schedulerObj;
        int coreId;
        static int coreIdIterator;
    
        Core(){}
        Core(UserData);
        double getNextCoreAvailableTime();
        serverStatus getCoreStatus();
        void setCoreStatus(serverStatus status);
        int addToThread(Event obj);
        Event* removeFromThread();
        int getBUsyThreadCount();
        void setBusyThreadCount(int count);
};

/**
 * @brief Construct a new Core:: Core object
 * 
 * @param obj 
 */
Core::Core(UserData obj){
    status = Idle;
    threadBusyCount=0;
    coreId=coreIdIterator++;
    schedulerObj = Scheduler(obj.policy);
}

/**
 * @brief returns next time stamp when the core is available
 * 
 * @return double 
 */
double Core::getNextCoreAvailableTime(){
    double nextFreeTime;
    if(schedulerObj.type == FCFS){
        if(threads.empty()==true)
            nextFreeTime = 0.0;
        else    
            nextFreeTime =  threads.back()->departureTime;
    }
    else{
        if(threads.empty()==true)
            nextFreeTime = 0.0;
        else    
            nextFreeTime =  max(threads.back()->departureTime, threads.back()->contextSwitchOutTime);
    }
    return nextFreeTime;
}
/**
 * @brief 
 * 
 * @return serverStatus 
 */
serverStatus Core::getCoreStatus(){
    if(this->threadBusyCount==MAX_THREAD_COUNT)
        return Busy;
    return Idle;
}

/**
 * @brief sets core status
 * 
 * @param status 
 */
void Core::setCoreStatus(serverStatus status){
    this->status = status;
}

/**
 * @brief  adds the event to the core process queue
 * 
 * @param obj 
 * @return int 
 */
int Core::addToThread(Event obj){
    threads.push(&obj);
    threadBusyCount = threadBusyCount + 1;
    if(threadBusyCount >= MAX_THREAD_COUNT)
        this->status = Busy;     
    return 1;
}

/**
 * @brief removes event entity from the core process queue
 * 
 * @return Event* 
 */
Event* Core::removeFromThread(){
    Event *obj = threads.front();
    threads.pop();
    this->threadBusyCount = this->threadBusyCount -1;
    if(this->threadBusyCount < MAX_THREAD_COUNT)
        this->status = Idle;             
    return obj;
}

/**
 * @brief get the number of thread busy on the core
 * 
 * @return int 
 */
int Core::getBUsyThreadCount(){
    return this->threadBusyCount;
}

/**
 * @brief set the busy therad count of core
 * 
 * @param count 
 */
void Core::setBusyThreadCount(int count){
    this->threadBusyCount+=count;
}

/**
 * @brief class represensts the Server with CORE_COUNT number of cores
 * 
 * Attributes   :   coreObj (Array of core Class instances)
 *              :   serviceTimeObj (instance of ServiceTime class)
 *              :   buffer (queue of events requried to store the events in case system is full)
 * 
 * Methods      :       Event getNextEvent()
 *              :       void addEventToBuffer(Event)
 *              :       serverStatus getServerStatus()
 *              :       Core getCore(int coreCount)
 *              :       int getFreeCore()
 *              :       bufferStatus getBufferStatus()
 */
class Server{
    public:
        Core coreObj[CORE_COUNT];
        Service_Time serviceTimeObj;
        std::queue<Event> buffer;

        Server(){}
        Server(UserData);
        Event getNextEvent();
        void addEventToBuffer(Event);
        serverStatus getServerStatus();
        Core getCore(int coreCount);
        int getFreeCore(); 
        bufferStatus getBufferStatus();

};

/**
 * @brief Construct a new Server:: Server object
 * 
 * @param obj 
 */
Server::Server(UserData obj){
    for(int i=0;i<CORE_COUNT;i++){
        coreObj[i]= Core(obj);
    }
    serviceTimeObj = Service_Time(obj.meanServiceTime,obj.serviceTimeDistribution);
}

// true == empty false = data in
/**
 * @brief get status of the server buffer queue
 * 
 * @return bufferStatus 
 */
bufferStatus Server::getBufferStatus(){
    if(this->buffer.empty())
        return Empty;
    if(this->buffer.size()==MAX_BUFFER_SIZE)
        return Full;
    return Available;
}
// coreId 0,1,2,3

/**
 * @brief get the core id of free core if any otherwise -1
 * 
 * @return int 
 */
int Server::getFreeCore(){
    int maxVal = MAX_THREAD_COUNT;
    int coreId = 0;
    for (int i=0;i<CORE_COUNT;i++){
        if(this->coreObj[i].getBUsyThreadCount()<maxVal){
            maxVal = this->coreObj[i].getBUsyThreadCount();
            coreId = this->coreObj[i].coreId;
        }
    }
    return coreId;    
}

/**
 * @brief adds event to the buffer queue
 * 
 * @param obj 
 */
void Server::addEventToBuffer(Event obj){
    this->buffer.push(obj);
}

/**
 * @brief removes the event from queue and returns
 * 
 * @return Event 
 */
Event Server::getNextEvent(){
    Event obj=  this->buffer.front();
    this->buffer.pop();
    return obj;
}

/**
 * @brief return server status
 * 
 * @return serverStatus 
 */
serverStatus Server::getServerStatus(){
    int busyCount=0;
    for (int i=0;i<CORE_COUNT;i++){
        if(this->coreObj[i].getCoreStatus()==Busy && this->coreObj[i].threadBusyCount==MAX_THREAD_COUNT){
            busyCount++;
        }
    }
    if(busyCount==CORE_COUNT)
        return Busy;

    return Idle;
}

/**
 * @brief return instance of the core requested
 * 
 * @param core // core id
 * @return Core 
 */
Core Server::getCore(int core){
    return this->coreObj[core];
}

/**
 * @brief struct to combine event time and corresponding event object
 * 
 */
struct timeEventTuple{
    double eventTime;
    Event eventObj;
};

/**
 * @brief function defination for comparision of ghe timeevent Tuple structure
 * 
 */
struct comparatorTimeEventTuple{
    bool operator()(struct timeEventTuple const&t1 , struct timeEventTuple const&t2){
        return t1.eventTime > t2.eventTime;
    }
};

/**
 * @brief handles all the request events generated in the system
 * Attributes       :       serverObj (Server class instance)
 *                  :       userDataObj(UserData class Instance)
 *                  :       gblSystemTime (double)
 *                  :       eventIdSeed (int ) used to generate event id's
 *                  :       userCount (int) states the current user count in the system
 *                  :       maxRequestCount (unsigned int) maximum number of requests being generated in the system
 *                  :       timeoutObj (Timeout class instance)
 *                  :       nextEventQueue (a priority queue to hold the list of next events)
 * 
 * Methods          :        void setUserData();
 *                  :        int genNewEventId();
 *                  :        timeEventTuple getNextEvent();
 *                  :        void manageEvent(Event event);
 *                  :        Server getServerObj();
 *                  :        void setEvent(double,Event);
 *                  :        void arrive(Event);
 *                  :        void depart(Event);
 *                  :        void contextSwitchIn(Event);
 *                  :        void contextSwitchOut(Event);
 *                  :        bool IsNextEventTimeBufferEmpty();
 *                  :        void report(Event);
 *                  :        void printState(timeEventTuple);

 */
class EventHandler {
    public:
        Server serverObj;
        UserData userDataObj;
        double gblSystemTime;
        int eventIdSeed ;
        int userCount;
        unsigned int maxRequestCount;
        priority_queue < timeEventTuple, vector<timeEventTuple>, comparatorTimeEventTuple > nextEventTime;
        Timeout timeoutObj;

        EventHandler(){}
        EventHandler(UserData);
        void setUserData();
        int genNewEventId();
        timeEventTuple getNextEvent();
        void manageEvent(Event event);
        Server getServerObj();
        void setEvent(double,Event);
        void arrive(Event);
        void depart(Event);
        void contextSwitchIn(Event);
        void contextSwitchOut(Event);
        bool IsNextEventTimeBufferEmpty();
        void report(Event);
        void printState(timeEventTuple);
};

/**
 * @brief returns status of the eventTime buffer
 * 
 * @return true  if buffer is empty
 * @return false  if not empty
 */
bool EventHandler::IsNextEventTimeBufferEmpty() {
        return this->nextEventTime.empty();
}

/**
 * @brief prints the current state of the system 
 * 
 * @param timeEventTuple
 */
void EventHandler::printState(timeEventTuple te) {
    cout << gblSystemTime << "\t\t";
    outTrace << gblSystemTime << "\t\t";
	
    cout << "[";
    outTrace << "[";
    for (int i = 0; i < 4; i++) {
        cout << this->serverObj.coreObj[i].getCoreStatus();
    }
    cout << "]\t"; outTrace << "]\t";


	if (this->serverObj.buffer.empty()) {
		cout << "Empty\t\t";
        outTrace << "Empty\t\t";
	}
	else {
        cout << serverObj.buffer.front().arrivalTime << "\t\t";
        outTrace << serverObj.buffer.front().arrivalTime << "\t\t";
    }
	 	
    switch(te.eventObj.type) {
        case arrival:
            cout << "Arrival      \t\t";
            outTrace << "Arrival      \t\t";
            break;
        case departure:
            cout << "Departure    \t\t";
            outTrace << "Departure    \t\t";
            break;
        case contextSwitchInEvent:
            cout << "Cntx Swtch In\t\t";
            outTrace << "Cntx Swtch In\t\t";
            break;
        case contextSwitchOutEvent:
            cout << "Cntx Swtch Out\t\t";
            outTrace << "Cntx Swtch Out\t\t";
            break; 
    }
    
    cout << "\t\t" << te.eventTime << "\t\t" << endl;
    outTrace << "\t\t" << te.eventTime << "\t\t" << endl;
	cout << "==================================================================================================" << endl;
    outTrace << "==================================================================================================" << endl;
}

/**
 * @brief logs the event details at exit 
 * 
 * @param Event 
 */
void EventHandler::report(Event e) {
    outdata << e.eventId << "," << e.arrivalTime << "," << e.departureTime << "," << e.waitingTime<< "," << e.departureTime - e.arrivalTime << "," << e.request_count << "," << e.response_count<<","<<e.objectId << endl;
}

/**
 * @brief Construct a new Event Handler:: Event Handler object
 * 
 * @param obj 
 */
EventHandler::EventHandler(UserData obj){
    serverObj = Server(obj);
    
    obj.noOfUsers = (obj.noOfUsers < MAX_USER_COUNT) ? obj.noOfUsers : MAX_USER_COUNT;
    userDataObj = obj;
    
    timeoutObj = Timeout(obj.meanTimeoutTime,obj.timeotTimeDistribution);
    eventIdSeed = 1;
    userCount=0;
    gblSystemTime=0;
    maxRequestCount = obj.noOfUsers * obj.maxRequestPerUser;
}

/**
 * @brief generates new event ID
 * 
 * @return int 
 */
int EventHandler::genNewEventId(){
    return this->eventIdSeed++;
}

/**
 * @brief returns next event in the event queue
 * 
 * @return timeEventTuple 
 */
timeEventTuple EventHandler::getNextEvent(){
    timeEventTuple obj = this->nextEventTime.top();
    this->nextEventTime.pop();
    return obj;
}

/**
 * @brief calls the appropriate function as per the state of the event
 * 
 * @param event 
 */
void EventHandler::manageEvent(Event event){

   switch (event.type)
   {
    case arrival:
        this->arrive(event);
        break;
        
    case departure:
        this->depart(event);
        break;

    case contextSwitchInEvent:
        contextSwitchIn(event);
        break;
    
    case contextSwitchOutEvent:
        contextSwitchOut(event);
        break;

    default:
        break;
   } 
}

/**
 * @brief adds event to the time event queue
 * 
 * @param timeOfEvent 
 * @param obj 
 */
void EventHandler::setEvent(double timeOfEvent, Event obj){
    this->nextEventTime.push({timeOfEvent,obj});
}

/**
 * @brief returns Instance of server class
 * 
 * @return Server 
 */
Server EventHandler::getServerObj(){
    return this->serverObj;
}

/**
 * @brief Arrival event handler
 * 
 * @param Event X
 */
void EventHandler::arrive(Event X){

    // creates new Event instances if not reached the required count
    if( userCount <= this->userDataObj.noOfUsers  ){

        int requestCount = this->genNewEventId();
        if(requestCount <= maxRequestCount){
            // make the Event object ready of arrival
            Event obj = Event();
            obj.objectId=++userCount;
            obj.arrivalTime = gblSystemTime+obj.getRandomThinkTime();
            obj.eventId = requestCount;
            obj.type = arrival;
            obj.serviceTime = this->serverObj.serviceTimeObj.getServiceTime();
            obj.eventServiceTime = this->serverObj.serviceTimeObj.getServiceTime();
            obj.timeout = gblSystemTime + this->timeoutObj.getTimeout();
            obj.request_count  =1;
            obj.response_count =0;
            //push the event object in the event queue
            this->setEvent(obj.arrivalTime,obj);  
        }
    }
    // in case server is busy push the event X in the wait buffer of the server
    if(this->serverObj.getServerStatus()==Busy){

        if(this->serverObj.getBufferStatus()==Full){
            request_drops++;
        }
        else {
            this->serverObj.addEventToBuffer(X);
        }
    }
    // if the server is not fully  busy , get a free core and assign the event to the core
    else{
        int freeCore =-1;
        freeCore = this->serverObj.getFreeCore();
        double nextAvailableTime = 0.0;
        nextAvailableTime = max(serverObj.coreObj[freeCore].getNextCoreAvailableTime(), gblSystemTime);
        X.core = freeCore;
        X.contextSwitchInTime = nextAvailableTime + CONTEXT_SWITCH_TIME;
        X.type = contextSwitchInEvent;
        serverObj.coreObj[freeCore].addToThread(X);
        X.thread = this->serverObj.coreObj[freeCore].getBUsyThreadCount();
        // send the event to the event queue
        this->setEvent(X.contextSwitchInTime,X);  
    }
} 

/**
 * @brief Departure event handle 
 * 
 * @param Event X 
 */
void EventHandler::depart(Event X){

    // removes the event from the core process thread
    this->serverObj.coreObj[X.core].removeFromThread();

    // get next request in the server buffer and assign it to a core
    if(this->serverObj.getBufferStatus() != Empty){
        Event A =  this->serverObj.getNextEvent();
        int freeCore = -1;
        freeCore = this->serverObj.getFreeCore();
        double nextAvialableTime = 0.0;
        nextAvialableTime = max(serverObj.coreObj[freeCore].getNextCoreAvailableTime(), gblSystemTime);
        A.core = freeCore;
        A.contextSwitchInTime = nextAvialableTime + CONTEXT_SWITCH_TIME;
        A.type = contextSwitchInEvent;
        serverObj.coreObj[freeCore].addToThread(A);
        A.thread = this->serverObj.coreObj[freeCore].getBUsyThreadCount();
        this->setEvent(A.contextSwitchInTime,A);
    }

    // if response is under the timeout of the request then generate new request
    if(X.departureTime < X.timeout){
        X.response_count++;
        report(X);
        int requestCount = this->genNewEventId();
        if(requestCount <= maxRequestCount){
            X.eventId = requestCount;
            X.type = arrival;
            X.timeout = gblSystemTime + this->timeoutObj.getTimeout();
            X.arrivalTime = gblSystemTime+X.getRandomThinkTime();
            X.serviceTime = this->serverObj.serviceTimeObj.getServiceTime();
            X.eventServiceTime = this->serverObj.serviceTimeObj.getServiceTime();
            X.departureTime = 0;
            X.waitingTime =0 ;
            X.core = 0;
            X.thread = 0;
            X.request_count +=1;
            this->setEvent(X.arrivalTime,X);  
        }
    }
    // if departure of event X happens after the timeout value the user will wont respond to the request and will regenrate the request
    else{
        X.type = arrival;
        X.arrivalTime = gblSystemTime+X.getRandomThinkTime();
        X.timeout = gblSystemTime + this->timeoutObj.getTimeout();
        X.departureTime = 0;
        X.waitingTime = 0;
        X.core = 0;
        X.thread = 0;
        X.request_count += 1;
        this->setEvent(X.arrivalTime,X);  
    }
}

/**
 * @brief Handles the Context Switch Out event
 * 
 * @param X 
 */
void EventHandler::contextSwitchOut(Event X){

    // remove the core from the process thread of the core
    this->serverObj.coreObj[X.core].removeFromThread();
    double nextAvailableTime = 0.0;
    nextAvailableTime = max(serverObj.coreObj[X.core].getNextCoreAvailableTime(), gblSystemTime);
    X.contextSwitchInTime = nextAvailableTime + CONTEXT_SWITCH_TIME;
    X.type = contextSwitchInEvent;
    this->serverObj.coreObj[X.core].addToThread(X);
    // assign it next context switch in time and push it back in the queue.
    this->setEvent(X.contextSwitchInTime, X);
}

/**
 * @brief handles the context Switch In event
 * 
 * @param X 
 */
void EventHandler::contextSwitchIn(Event X){

    // if the cervice time left is more than time quantum then assign next context switch out time
    if (X.serviceTime > TIME_QUANTUM) {
        X.serviceTime -= TIME_QUANTUM;
        X.contextSwitchOutTime = gblSystemTime + TIME_QUANTUM;
        X.type = contextSwitchOutEvent;
        this->setEvent(X.contextSwitchOutTime, X);
    }
    // of the remaining service time is less than the time quantum assign the departure time 
    else {
        X.departureTime = gblSystemTime + X.eventServiceTime;
        X.waitingTime = (X.departureTime - X.arrivalTime) - X.eventServiceTime;
        meanWaitingTime += X.waitingTime;
        meanResponseTime += X.departureTime - X.arrivalTime; 
        X.type = departure;
        this->setEvent(X.departureTime,X);
    } 
}

/**
* @brief Handles the simulation
* Attributes   :    eventHandlerObj (Event Handler instance)
* Methods      :     void time(timeEventTuple X)
*              :     void initialize()
*/
class Simulation {
    public:
        EventHandler eventHandlerObj;
        Simulation(){}
        Simulation(UserData);
        void time(timeEventTuple X);
        void initialize();
};

/**
 * @brief Construct a new Simulation:: Simulation object
 * 
 * @param obj 
 */
Simulation::Simulation(UserData obj){
    eventHandlerObj = EventHandler(obj);
};

/**
 * @brief sets the global system time
 * 
 * @param X 
 */
void Simulation::time(timeEventTuple X){
    eventHandlerObj.gblSystemTime = X.eventTime;
}

/**
 * @brief Initialize the simulation by pushing teh first request Event
 * 
 */
void Simulation::initialize(){
    Event obj = Event();
    obj.eventId = eventHandlerObj.genNewEventId();
    obj.arrivalTime = eventHandlerObj.gblSystemTime;
    obj.type = arrival;
    obj.timeout = eventHandlerObj.gblSystemTime + eventHandlerObj.timeoutObj.getTimeout();
    obj.serviceTime = eventHandlerObj.serverObj.serviceTimeObj.getServiceTime();
    obj.eventServiceTime = eventHandlerObj.serverObj.serviceTimeObj.getServiceTime();
    obj.contextSwitchInTime = 0.0;
    obj.contextSwitchOutTime = 0.0;
    obj.request_count=1;
    obj.objectId = ++eventHandlerObj.userCount;
    obj.response_count =0;
    eventHandlerObj.userCount++;
   eventHandlerObj.setEvent(0, obj);
}

// core Id Iterator Initialization
int Core::coreIdIterator = 0;

/**
 * @brief reads input from file 
 * 
 * @param obj 
 */
void readFromFile (UserData obj) {
    string file_name = "";
    cout << "Enter file name" << endl;
    cin >> file_name;
    ifstream indata; 
    int num; 
    indata.open(file_name); 
    if(!indata) { 
        cerr << "Error: file could not be opened" << endl;
        exit(1);
    }
    indata >> obj.meanServiceTime;
    indata >> obj.meanTimeoutTime;
    indata >> num;
    switch(num) {
        case Exponential:
            obj.serviceTimeDistribution = Exponential;
            break;
        case Uniform:
            obj.serviceTimeDistribution = Uniform;
            break;
        case Constant:
            obj.serviceTimeDistribution = Constant;
            break;
        default:
            cout << "Wrong Distribution" << endl;
            exit(0);
            break;
    }
    indata >> num;
    switch(num) {
        case Exponential:
            obj.timeotTimeDistribution = Exponential;
            break;
        case Uniform:
            obj.timeotTimeDistribution = Uniform;
            break;
        case Constant:
            obj.timeotTimeDistribution = Constant;
            break;
        default:
            cout << "Wrong Distribution" << endl;
            exit(0);
            break;
    }
    indata >> obj.noOfUsers;
    indata >> obj.maxRequestPerUser;
    indata >> num;
    switch(num) {
        case FCFS:
            obj.policy = FCFS;
            break;
        case roundRobin:
            obj.policy = roundRobin;
            break;
        default:
            cout << "Wrong Policy";
            break;
    }
    indata.close();
    // return obj;
}

/**
 * @brief manual read of input from the user
 * 
 * @param obj 
 */
void manualRead(UserData obj) {
    cout << "Enter mean service time of server" << endl;
    cin >> obj.meanServiceTime;
    cout << "Enter mean Timeout time of request" << endl;
    cin >> obj.meanTimeoutTime;
    cout << "Enter service time distribution eg. 1" << endl;
    cout << "1. Exponential" << endl << "2. Uniform" << endl << "3. Constant" << endl;
    int a = 0;
    cin >> a;
    switch(a) {
        case Exponential:
            obj.serviceTimeDistribution = Exponential;
            break;
        case Uniform:
            obj.serviceTimeDistribution = Uniform;
            break;
        case Constant:
            obj.serviceTimeDistribution = Constant;
            break;
        default:
            cout << "Wrong Distribution" << endl;
            exit(0);
            break;
    }
    cout << "Enter Timeout distribution" << endl;
    cout << "1. Exponential" << endl << "2. Uniform" << endl << "3. Constant" << endl << "Enter numbers" << endl;
    cin >> a;
    switch(a) {
        case Exponential:
            obj.timeotTimeDistribution = Exponential;
            break;
        case Uniform:
            obj.timeotTimeDistribution = Uniform;
            break;
        case Constant:
            obj.timeotTimeDistribution = Constant;
            break;
        default:
            cout << "Wrong Distribution" << endl;
            exit(0);
            break;
    }
    cout << "Enter number of users in the system" << endl;
    cin >> obj.noOfUsers;
    cout << "Enter number of requests per user in the system" << endl;
    cin >> obj.maxRequestPerUser;
    cout << "Enter scheduling policy" << endl << "1. FCFS" << endl << "2. Round Robin" << endl;
    cin >> a;
    switch(a) {
        case FCFS:
            obj.policy = FCFS;
            break;
        case roundRobin:
            obj.policy = roundRobin;
            break;
        default:
            cout << "Wrong Policy";
            break;
    }
}

/**
 * @brief read user input
 * 
 * @param obj 
 */
void read(UserData obj) {
    cout << "Enter input type eg. 1" << endl << "1. Manual" << endl << "2. From a file" << endl;
    int input_type = 0;
    cin >> input_type;

    if (input_type == 1) {
        manualRead(obj);
    }
    else {
        readFromFile(obj);
    } 
    // return obj;
}

/**
 * @brief Main Function
 * 
 * @return int 
 */
int main(){
    UserData obj = UserData();
    //read(obj);
    obj.meanServiceTime = 2;
    obj.meanTimeoutTime =10;
    obj.serviceTimeDistribution = Exponential;
    obj.timeotTimeDistribution = Uniform;
    obj.noOfUsers = 5;
    obj.maxRequestPerUser = 8;
    obj.policy = roundRobin;
    
    outdata.open("outfile.csv");
    outTrace.open("Trace.txt");

    outdata << "Even_Id,Arrival_Time,Departure_Time,Waiting_Time,Response_Time,Request_Count,Response_Count,Object_Id" << endl;

    Simulation simObj = Simulation(obj);
    simObj.initialize();
    
    cout << "Global System Time\t" << "Core Status\t" << "Server Buffer Top Element\t" << "Next Event Type\t" << "Next Event Time" << endl;

    while(simObj.eventHandlerObj.IsNextEventTimeBufferEmpty()==false){
        timeEventTuple x = simObj.eventHandlerObj.getNextEvent();
        simObj.eventHandlerObj.printState(x);
        simObj.time(x);
        simObj.eventHandlerObj.manageEvent(x.eventObj);
    }

    outTrace << "Mean Waiting Time = " << meanWaitingTime/simObj.eventHandlerObj.eventIdSeed << endl;
    outTrace << "Mean Response Time = " << meanResponseTime/simObj.eventHandlerObj.eventIdSeed << endl;
    outTrace << "Total Request Drops = " << request_drops << endl;
    cout << "Total Request Drops = " << request_drops << endl;
    cout << "Mean Waiting Time = " << meanWaitingTime/simObj.eventHandlerObj.eventIdSeed << endl;
    cout << "Mean Response Time = " << meanResponseTime/simObj.eventHandlerObj.eventIdSeed << endl;
    return 0;
}