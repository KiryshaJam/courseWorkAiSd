#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>

using namespace std;

const int NEED_TO_GALOP = 7;
const int MAX_VERTICES = 50; // Максимальное количество вершин

// Структура для ребра
struct Edge {
    int u, v;
    int weight;
    bool operator<(const Edge& other) const {
        return weight < other.weight;
    }
};

class DynamicArray {
private:
    Edge* array; // Теперь это массив объектов типа Edge
    int capacity;
    int size;

    void resize(int newCapacity) {
        Edge* newArray = new Edge[newCapacity];
        for (int i = 0; i < size; ++i) {
            newArray[i] = array[i];
        }
        delete[] array;
        array = newArray;
        capacity = newCapacity;
    }

public:
    DynamicArray() {
        array = new Edge[2];
        capacity = 2;
        size = 0;
    }

    ~DynamicArray() {
        delete[] array;
    }

    void addElement(const Edge& element) {
        if (size == capacity) {
            resize(capacity * 2);
        }
        array[size] = element;
        ++size;
    }

    int getSize() const {
        return size;
    }

    Edge& operator[](int index) {
        if (index >= 0 && index < size) {
            return array[index];
        } else {
            cout << "Индекс вне диапазона!" << endl;
            exit(1);  // Прерывание программы в случае ошибки
        }
    }

    void printArray() const {
        std::cout << "Элементы массива: ";
        for (int i = 0; i < size; ++i) {
            std::cout << "(" << array[i].u << ", " << array[i].v << ") ";
        }
        std::cout << std::endl;
    }

    void deleteArray() {
        delete[] array;
        array = nullptr;
        size = 0;
        capacity = 0;
    }
};

// Структура для очереди
class Queue {
private:
    int* array;
    int capacity;
    int size;
    int front;
    int rear;

    void resize(int newCapacity) {
        int* newArray = new int[newCapacity];
        int j = 0;
        for (int i = front; i < size; i++) {
            newArray[j++] = array[i % capacity];
        }
        delete[] array;
        array = newArray;
        front = 0;
        rear = j;
        capacity = newCapacity;
    }

public:
    Queue() {
        capacity = 2;
        size = 0;
        front = 0;
        rear = 0;
        array = new int[capacity];
    }

    ~Queue() {
        delete[] array;
    }

    void enqueue(int value) {
        if (size == capacity) {
            resize(capacity * 2);
        }
        array[rear] = value;
        rear = (rear + 1) % capacity;
        size++;
    }

    int dequeue() {
        if (size == 0) {
            cout << "Очередь пуста!" << endl;
            exit(1);
        }
        int value = array[front];
        front = (front + 1) % capacity;
        size--;
        return value;
    }

    bool isEmpty() const {
        return size == 0;
    }
};

// Функция сортировки вставками
void insertionSort(DynamicArray& arr, int left, int right) {
    for (int i = left + 1; i <= right; i++) {
        Edge temp = arr[i];
        int j = i - 1;
        
        while (j >= left && arr[j].weight > temp.weight) {
            arr[j + 1] = arr[j];
            j--;
        }
        arr[j + 1] = temp;
    }
}

int gallopSearchLeft(DynamicArray& arr, int value, int start, int end) {
    while (start < end) {
        int mid = (start + end) / 2;
        if (arr[mid].weight < value) {
            start = mid + 1;
        } else {
            end = mid;
        }
    }
    return start;
}

int gallopSearchRight(DynamicArray& arr, int value, int start, int end) {
    while (start < end) {
        int mid = (start + end) / 2;
        if (arr[mid].weight <= value) {
            start = mid + 1;
        } else {
            end = mid;
        }
    }
    return start;
}

int calculateMinRun(int n) {
    int r = 0;
    while (n >= 64) {
        r |= (n & 1);
        n >>= 1;
    }
    return n + r;
}

void merge(DynamicArray& arr, int left, int mid, int right) {
    int len1 = mid - left + 1;
    int len2 = right - mid;

    DynamicArray leftArr, rightArr;

    for (int i = 0; i < len1; i++) {
        leftArr.addElement(arr[left + i]);
    }
    for (int i = 0; i < len2; i++) {
        rightArr.addElement(arr[mid + 1 + i]);
    }

    int i = 0, j = 0, k = left;
    int gallopLeft = 0, gallopRight = 0;

    while (i < len1 && j < len2) {
        if (leftArr[i].weight <= rightArr[j].weight) {
            arr[k++] = leftArr[i++];
            gallopRight = 0;
            if (++gallopLeft >= NEED_TO_GALOP) {
                int newIdx = gallopSearchLeft(rightArr, leftArr[i].weight, j, len2);
                while (j < newIdx) arr[k++] = rightArr[j++];
                gallopLeft = 0;
            }
        } else {
            arr[k++] = rightArr[j++];
            gallopLeft = 0;
            if (++gallopRight >= NEED_TO_GALOP) {
                int newIdx = gallopSearchRight(leftArr, rightArr[j].weight, i, len1);
                while (i < newIdx) arr[k++] = leftArr[i++];
                gallopRight = 0;
            }
        }
    }

    while (i < len1) arr[k++] = leftArr[i++];
    while (j < len2) arr[k++] = rightArr[j++];
}

void timSort(DynamicArray& arr) {
    int n = arr.getSize();
    int minrun = calculateMinRun(n);

    for (int i = 0; i < n; i += minrun) {
        int right = std::min(i + minrun - 1, n - 1);
        insertionSort(arr, i, right);
    }

    for (int size = minrun; size < n; size = 2 * size) {
        for (int left = 0; left < n; left += 2 * size) {
            int mid = left + size - 1;
            int right = std::min(left + 2 * size - 1, n - 1);

            if (mid < right) {
                merge(arr, left, mid, right);
            }
        }
    }
}

// Структура для системы непересекающихся множеств (Union-Find)
struct UnionFind {
    int* parent;
    int* rank;
    int size;

    UnionFind(int n) {
        size = n;
        parent = new int[size];
        rank = new int[size];
        for (int i = 0; i < size; ++i) {
            parent[i] = i;
            rank[i] = 0;
        }
    }

    ~UnionFind() {
        delete[] parent;
        delete[] rank;
    }

    int find(int u) {
        if (parent[u] != u)
            parent[u] = find(parent[u]);
        return parent[u];
    }

    bool union_sets(int u, int v) {
        int root_u = find(u);
        int root_v = find(v);
        if (root_u != root_v) {
            if (rank[root_u] < rank[root_v])
                parent[root_u] = root_v;
            else if (rank[root_u] > rank[root_v])
                parent[root_v] = root_u;
            else {
                parent[root_v] = root_u;
                rank[root_u]++;
            }
            return true;
        }
        return false;
    }
};

// Функция для выполнения алгоритма Краскала
int kruskalAlgorithm(int n, DynamicArray& edges, const string* vertices) {
    int mst_weight = 0;
    UnionFind uf(n);
    DynamicArray mst_edges;

    for (int i = 0; i < edges.getSize(); ++i) {
        const Edge& edge = edges[i];
        if (uf.union_sets(edge.u, edge.v)) {
            mst_edges.addElement(edge);
            mst_weight += edge.weight;
        }
    }

    // Выводим результат
    for (int i = 0; i < mst_edges.getSize(); ++i) {
        const Edge& edge = mst_edges[i];
        cout << vertices[edge.u] << " " << vertices[edge.v] << endl;
    }
    return mst_weight;
}

// Функция для обхода в глубину (DFS)
void DFS(int v, bool* visited, const int graph[MAX_VERTICES][MAX_VERTICES], int n) {
    visited[v] = true;
    cout << v << " ";
    
    for (int i = 0; i < n; ++i) {
        if (graph[v][i] && !visited[i]) {
            DFS(i, visited, graph, n);
        }
    }
}

// Функция для обхода в ширину (BFS)
void BFS(int start, const int graph[MAX_VERTICES][MAX_VERTICES], int n) {
    bool visited[MAX_VERTICES] = {false};
    Queue q;
    visited[start] = true;
    q.enqueue(start);

    while (!q.isEmpty()) {
        int v = q.dequeue();
        cout << v << " ";

        for (int i = 0; i < n; ++i) {
            if (graph[v][i] && !visited[i]) {
                visited[i] = true;
                q.enqueue(i);
            }
        }
    }
}

// Меню для выбора действия
void menu(int n, DynamicArray& edges, const string* vertices, const int graph[MAX_VERTICES][MAX_VERTICES]) {
    int choice;
    bool visited[MAX_VERTICES] = {false};
    int mst_weight = 0;

    do {
        cout << "\nМеню:\n";
        cout << "1. Выполнить алгоритм Краскала\n";
        cout << "2. Выполнить обходы графа\n";
        cout << "3. Выйти\n";
        cout << "Введите номер действия: ";
        cin >> choice;

        switch (choice) {
            case 1:
                system("clear");
                timSort(edges);
                mst_weight = kruskalAlgorithm(n, edges, vertices);
                cout << "Вес минимального остова: " << mst_weight << endl;
                break;

            case 2:
                system("clear");
                cout << "Выберите обход:\n";
                cout << "1. Обход в глубину (DFS)\n";
                cout << "2. Обход в ширину (BFS)\n";
                int sub_choice;
                cin >> sub_choice;

                if (sub_choice == 1) {
                    system("clear");
                    cout << "DFS обход с вершины 0: ";
                    DFS(0, visited, graph, n);
                    cout << endl;
                } else if (sub_choice == 2) {
                    system("clear");
                    cout << "BFS обход с вершины 0: ";
                    BFS(0, graph, n);
                    cout << endl;
                }
                break;

            case 3:
                break;

            default:
                cout << "Некорректный выбор. Попробуйте снова.\n";
        }

    } while (choice != 3);
}

int main() {
    system("clear");
    ifstream inputFile("graph.txt");  // Открытие файла с графом
    if (!inputFile) {
        cerr << "Ошибка при открытии файла!" << endl;
        return 1;
    }

    string line;
    string vertices[MAX_VERTICES];  // Динамический массив для вершин
    int n = 0;  // Количество вершин

    // Считываем список вершин
    if (getline(inputFile, line)) {
        stringstream ss(line);
        while (ss >> vertices[n] && n < MAX_VERTICES) {
            ++n;
        }
    }

    int graph[MAX_VERTICES][MAX_VERTICES];  // Матрица смежности

    // Считываем матрицу смежности
    for (int i = 0; i < n; ++i) {
        if (getline(inputFile, line)) {
            stringstream ss(line);
            for (int j = 0; j < n; ++j) {
                ss >> graph[i][j];
            }
        }
    }

    DynamicArray edges;

    // Формируем список рёбер
    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            if (graph[i][j] > 0) {
                Edge edge = {i, j, graph[i][j]};
                edges.addElement(edge);
            }
        }
    }

    // Вызываем меню
    menu(n, edges, vertices, graph);

    return 0;
}
