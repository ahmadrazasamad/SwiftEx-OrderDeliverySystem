#include <iostream>
#include <string>
#include <vector> // for adjacency_list
#include <algorithm>
#include <queue>         // for graph
#include <limits>        // for numeric_limits
#include <unordered_map> // for graph
using namespace std;

struct Order
{
    int priority;
    string productName, customerName, destination;
    double distanceToDestination;
};

struct Edge
{
    int destination; // index of the destination location in the graph
    double distance; // distance to the destination location in kilometers
};

const vector<string> locationNames = {"Nazimabad", "Purani Sabzi Mandi", "Mazar-e-Quaid", "Soldier Bazar", "Naz Plaza", "Empress Market", "Saddar", "Regal Chowk", "Akbar Road", "Burns Road", "Aram Bagh", "Gari Khata", "Nanak Warah", "Lea Market", "Kharadar", "Lyari", "Bhimpura", "Gosht Market", "Paper Market", "Pakistan Chowk"};
vector<vector<Edge>> map(locationNames.size()); // 20 locations, adjacency list

string currentDestinationOfRider = "Saddar"; // center of the city is the starting point as the karachi branch of SwiftEx is located in here
string previousDestinationOfRider = "";

bool isProcessingPriorityOrders = false; // will be useful in buildHeapTree, deleteHeap and bill calculations

// ================ helper functions ================
int getLocationIndex(const string &location)
{
    for (int i = 0; i < locationNames.size(); ++i)
    {
        if (locationNames[i] == location)
            return i;
    }
    return -1;
}

double determineDistance(Order order, string start, string end)
{ // will determine the shortest distance, dijikistra
    int src = getLocationIndex(start);
    int dest = getLocationIndex(end);

    if (src == dest) // If the rider's starting point is also it's destination
        return 0.0;

    vector<double> distances(map.size(), numeric_limits<double>::max());
    distances[src] = 0.0;

    priority_queue<pair<double, int>, vector<pair<double, int>>, greater<pair<double, int>>> pq;
    pq.push({0.0, src});

    while (!pq.empty())
    {
        pair<double, int> top = pq.top();
        double currentDistance = top.first;
        int current = top.second;
        pq.pop();

        if (current == dest)
            return currentDistance;

        for (const Edge &edge : map[current])
        {
            double newDistance = currentDistance + edge.distance;
            if (newDistance < distances[edge.destination])
            {
                distances[edge.destination] = newDistance;
                pq.push({newDistance, edge.destination});
            }
        }
    }

    return numeric_limits<double>::max(); // If no path exists, infinity
}

void billCalculationAndPrinting(Order order)
{ // bill will be calculated with respect to the rider's current location to the customer's destination
    double bill = 0;
    if (currentDestinationOfRider == previousDestinationOfRider) // means the rider's starting point is also it's destination, therefore only 50 rupees is delivery charges
        bill = 50;
    else
    {
        double costOfOneLiterPetrol = 260.96;        // in Pakistan as of 26 august 2024
        double distanceCoveredInOneLiterPetrol = 35; // for a 70cc bike
        double totalPetrolConsumed = order.distanceToDestination / distanceCoveredInOneLiterPetrol;
        bill = totalPetrolConsumed * costOfOneLiterPetrol;
        bill += 50; // delivery charges
    }
    if (isProcessingPriorityOrders && order.priority != 1)
    {
        bill += 50; // priority orders are charged 50 extra
        if (order.priority == 3)
            bill += 50; // 50 extra for immediate delivery
    }
    cout << "Your total bill is " << bill << "/-\n";
}

string trim(const string &str)
{
    size_t first = str.find_first_not_of(" \t\n\r");
    size_t last = str.find_last_not_of(" \t\n\r");
    if (first == string::npos || last == string::npos)
        return ""; // If the string contains only whitespace characters
    return str.substr(first, last - first + 1);
}

void getOrderDetails(Order &order)
{
    string customerName = "";
    cout << "Enter the customer name: ";
    getline(cin, customerName);
    customerName = trim(customerName);

    while (customerName.empty())
    {
        cout << "Customer's name cannot be empty or contain whitespaces only! Kindly re-enter the name of the customer again: ";
        getline(cin, customerName);
        customerName = trim(customerName);
    }
    order.customerName = customerName;

    string productName = "";
    cout << "Enter the name of the product of the customer: ";
    getline(cin, productName);
    productName = trim(productName);

    while (productName.empty())
    {
        cout << "Product name cannot be empty or contain whitespaces only! Kindly re-enter the name of the product again: ";
        getline(cin, productName);
        productName = trim(productName);
    }
    order.productName = productName;

    cout << "Choose one location from the following to transport the product '" << order.productName << "' to:\n";
    for (int i = 0; i < locationNames.size(); i++) // printing locations
        cout << (i + 1) << ") " << locationNames[i] << endl;
    int locationChoice = 0;
    cout << "Enter your choice: ";
    while (true)
    { // checker for invalid choice
        cin >> locationChoice;

        if (cin.fail() || locationChoice < 1 || locationChoice > locationNames.size())
        {
            cout << "Invalid location choice entered.\nKindly re-enter(1-" << locationNames.size() << "): ";
            cin.clear(); // Clear the error flag set by invalid input
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }
        else
        {
            cin.ignore();
            break;
        }
    }
    order.destination = locationNames[locationChoice - 1];
}

void heapify(Order orders[], int n, int i, bool isMaxHeap)
{
    int extreme = i;
    int left = 2 * i + 1;
    int right = 2 * i + 2;

    if (isMaxHeap)
    {
        if (left < n && orders[extreme].priority < orders[left].priority)
            extreme = left;
        if (right < n && orders[extreme].priority < orders[right].priority)
            extreme = right;
    }
    else
    {
        if (left < n && orders[extreme].distanceToDestination > orders[left].distanceToDestination)
            extreme = left;
        if (right < n && orders[extreme].distanceToDestination > orders[right].distanceToDestination)
            extreme = right;
    }

    if (extreme != i)
    {
        swap(orders[i], orders[extreme]);
        heapify(orders, n, extreme, isMaxHeap);
    }
}

void buildHeap(Order orders[], int n, bool isMaxHeap)
{
    for (int i = (n / 2) - 1; i >= 0; i--)
        heapify(orders, n, i, isMaxHeap);
}


Order deleteFromHeap(Order orders[], int &lastAddedElementIndex)
{
    if (lastAddedElementIndex == -1)
    {
        cout << "The rider has no orders assigned! Kindly assign some orders to the rider first.";
        Order defaultOrder;
        return defaultOrder;
    }
    Order deliveredItem = orders[0];
    cout << "Mr/Miss " << deliveredItem.customerName << "'s order: '" << deliveredItem.productName << "' has sucessfully been delivered to the destination: '" << deliveredItem.destination << "'.\n";

    // Updation in the destination of the rider
    previousDestinationOfRider = currentDestinationOfRider;
    currentDestinationOfRider = deliveredItem.destination; // the rider's current location will be updated as product has been delivered to the location

    orders[0] = orders[lastAddedElementIndex];
    lastAddedElementIndex--;
    if (isProcessingPriorityOrders)
        heapify(orders, lastAddedElementIndex + 1, 0, true);
    else
    {
        for (int i = 0; i <= lastAddedElementIndex; i++)
            orders[i].distanceToDestination = determineDistance(orders[i], currentDestinationOfRider, orders[i].destination);
        heapify(orders, lastAddedElementIndex + 1, 0, false);
    }
    return deliveredItem;
}

void processPriorityOrDistanceOrders(Order orders[], int n, int &lastAddedElementIndex)
{
    cout << "\t\t\t\tThe rider delivering the orders (processing orders).\n";
    for (int i = 1; i <= n; i++)
    { // processing 15 orders
        Order processedOrder = deleteFromHeap(orders, lastAddedElementIndex);
        if (isProcessingPriorityOrders) // as in the case of distance, distance is pre-determined
            processedOrder.distanceToDestination = determineDistance(processedOrder, previousDestinationOfRider, currentDestinationOfRider);
        billCalculationAndPrinting(processedOrder);
    }
}
// =======================================================================

// ================ scheme a ================ (priority)
void addPriorityOrdersInHeap(Order orders[], int n, int &lastAddedElementIndex)
{
    isProcessingPriorityOrders = true;
    cout << "\t\t\t\tAssigning orders to the rider (adding orders)...\n";
    for (int i = 1; i <= n; i++)
    { // entering (enqueue) 15 orders
        cout << "Order " << i << endl;

        Order order;
        getOrderDetails(order);

        int priority = 0;
        cout << "Enter priority of the order by choosing one of the options below:\n1. Normal\n2. High\n3. Immediate\nEnter your choice: ";
        while (true)
        { // checker for invalid choice
            cin >> priority;

            if (cin.fail() || priority < 1 || priority > 3)
            {
                cout << "Invalid choice entered. Kindly re-enter(1-3): ";
                cin.clear(); // Clear the error flag set by invalid input
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
            }
            else
            {
                cin.ignore();
                break;
            }
        }
        order.priority = priority;

        orders[++lastAddedElementIndex] = order;
        cout << "Order of product: '" << order.productName << "' assigned to the rider successfully.\n\n";
    }
    buildHeap(orders, n, true);
}
// =======================================================================

// ================ scheme b ================ (normal way, as they were ordered)
void enqueue(Order orderQueue[], Order order, int &front, int &rear, int n)
{
    if (front == (n - 1))
    {
        cout << "Queue is full, cannot assign more than " << n << " orders to a rider.\n";
        return;
    }
    if (front == -1)
        front = 0;
    rear++;
    orderQueue[rear] = order;
    cout << "Order of product: '" << order.productName << "' assigned to the rider successfully.\n\n";
}

void addOrdersInQueue(Order orderQueue[], int n, int &front, int &rear)
{
    cout << "\t\t\t\tAssigning orders to the rider (adding orders)...\n";
    for (int i = 1; i <= n; i++)
    { // entering (enqueue)
        cout << "Order " << i << endl;

        Order order;
        getOrderDetails(order);
        order.priority = 1; // priority of normal order is 1

        enqueue(orderQueue, order, front, rear, n);
    }
}

Order dequeue(Order orderQueue[], int &front, int rear)
{
    if ((front == -1) || (front == (rear + 1)))
    {
        cout << "The rider has no orders assigned! Kindly assign some orders to the rider first.";
        Order defaultOrder;
        return defaultOrder;
    }
    Order deliveredItem = orderQueue[front];
    cout << "Mr/Miss " << deliveredItem.customerName << "'s order: '" << deliveredItem.productName << "' has sucessfully been delivered to the destination: '" << deliveredItem.destination << "'.\n";

    // Updation in the destination of the rider
    previousDestinationOfRider = currentDestinationOfRider;
    currentDestinationOfRider = deliveredItem.destination; // the rider's current location will be updated as product has been delivered to the location

    front++;
    return deliveredItem;
}

void processQueueOrders(Order orderQueue[], int n, int &front, int &rear)
{
    cout << "\t\t\t\tThe rider delivering the orders (processing orders).\n";
    for (int i = 1; i <= n; i++)
    { // processing orders
        Order processedOrder = dequeue(orderQueue, front, rear);
        processedOrder.distanceToDestination = determineDistance(processedOrder, previousDestinationOfRider, currentDestinationOfRider);
        billCalculationAndPrinting(processedOrder);
    }
}
// =======================================================================

// ================ scheme c ================ (normal orders, w.r.t order destination from rider's current destination)
void addDistanceOrdersInHeap(Order orders[], int n, int &lastAddedElementIndex)
{
    cout << "\t\t\t\tAssigning orders to the rider (adding orders)...\n";
    for (int i = 1; i <= n; i++)
    { // entering (enqueue) 15 orders
        cout << "Order " << i << endl;

        Order order;
        getOrderDetails(order);
        order.priority = 1; // since normal order

        orders[++lastAddedElementIndex] = order;
        cout << "Order of product: '" << order.productName << "' assigned to the rider successfully.\n\n";
    }
    for (int i = 0; i <= lastAddedElementIndex; i++)
        orders[i].distanceToDestination = determineDistance(orders[i], currentDestinationOfRider, orders[i].destination);
    buildHeap(orders, n, false);
}
// =======================================================================

// ======================== Graph Initialization ========================
void addEdge(int src, int dest, double distance)
{ // For bidirectional map
    map[src].push_back({dest, distance});
    map[dest].push_back({src, distance});
}

void initializeMapOfKarachi()
{
    addEdge(0, 1, 5.5); // nazimabad to purani sabzi mandi
    addEdge(0, 3, 4.2); // nazimabad to soldier bazar

    addEdge(1, 2, 4.4); // purani sabzi mandi to mazar e quaid

    addEdge(2, 3, 1.6); // mazar e quaid to soldier bazar
    addEdge(2, 4, 2.1); // mazar e quaid to naz plaza
    addEdge(2, 5, 2.5); // mazar e quaid to empress market

    addEdge(3, 4, 2.0);  // soldier bazar to naz plaza
    addEdge(3, 6, 2.5);  // soldier bazar to saddar
    addEdge(3, 12, 4.5); // soldier bazar to nanak wara
    addEdge(3, 16, 4.9); // soldier bazar to bhimpura

    addEdge(4, 6, 0.900); // naz plaza to saddar

    addEdge(5, 6, 0.350); // empress market to saddar

    addEdge(6, 7, 1.4);  // saddar to regal chowk
    addEdge(6, 11, 1.9); // saddar to gari khata
    addEdge(6, 12, 3.2); // saddar to nanak warah

    addEdge(7, 8, 0.600); // regal chowk to akbar road

    addEdge(8, 9, 1.1); // akbar road to burns road

    addEdge(9, 10, 0.600); // burns road to Aram Bagh

    addEdge(10, 11, 0.700); // aram bagh to gari khata

    addEdge(11, 19, 1.1); // gari khata to pakistan chowk
    addEdge(11, 12, 1.4); // gari khata to nanak warah

    addEdge(12, 18, 1.2);   // nanak warah to paper market
    addEdge(12, 17, 0.160); // nanak warah to gosht market
    addEdge(12, 13, 0.750); // nanak warah to lea market
    addEdge(12, 16, 0.550); // nanak warah to bhimpura

    addEdge(13, 14, 1.5);   // lea market to kharadar
    addEdge(13, 16, 0.700); // lea market to bhimpura

    addEdge(14, 15, 4.4); // kharadar to lyari

    addEdge(15, 16, 2.8); // lyari to bhimpura

    addEdge(16, 17, 0.400); // bhimpura to gosht market

    // 17 gosht market's connections already made
    addEdge(18, 19, 0.550); // paper market to pakistan chowk
    // 19 Pakistan chowk's connections already made
}
// =======================================================================

int main()
{
    cout << "\t\t\t\t\tWelcome to SwiftEx.\n";
    initializeMapOfKarachi();

    const int n = 15; // maximux 15 orders can be assigned to the rider in one day
    Order orders[n];

    int choice = 0;
    cout << "Hi manager how would you like to manage (add & process) the "<< n << " orders today, select from the folowing options:\n1. Process urgent orders separately on immediate basis.\n2. Deliver normal orders to the customers in the same way as they were ordered.\n3. Deliver normal orders to the customers with respect to order delivery destination from rider's destination (smallest distance first).\nEnter your choice: ";

    while (true)
    {
        cin >> choice;

        if (cin.fail() || choice < 1 || choice > 3)
        {
            cout << "Invalid choice entered.\n Kindly re-enter: ";
            cin.clear(); // Clear the error flag set by invalid input
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }
        else
        {
            cin.ignore();
            cout << endl;
            break;
        }
    }

    switch (choice)
    {
    case 1:
    {
        int lastAddedElementIndexPriority = -1; // will start entering from 0th index in the heap
        addPriorityOrdersInHeap(orders, n, lastAddedElementIndexPriority);
        processPriorityOrDistanceOrders(orders, n, lastAddedElementIndexPriority);
        break;
    }
    case 2:
    {
        int front = -1;
        int rear = -1;
        addOrdersInQueue(orders, n, front, rear);
        processQueueOrders(orders, n, front, rear);
        break;
    }
    case 3:
    {

        int lastAddedElementIndexDistance = -1; // will start entering from 0th index in the heap
        addDistanceOrdersInHeap(orders, n, lastAddedElementIndexDistance);
        processPriorityOrDistanceOrders(orders, n, lastAddedElementIndexDistance);
        break;
    }
    }
    return 0;
}