#include <iostream>
#include <vector>

using namespace std;

int binarySearch(const vector<int>& arr, int target) {
    int low = 0;
    int high = arr.size() - 1;
    vector<int> path; // Store visited indices

    while (low <= high) {
        // --- Changed this line to pick the "upper middle" ---
        int mid = (low + high + 1) / 2;

        path.push_back(mid); // Store the index visited

        if (arr[mid] == target) {
            // Print path
            cout << "Path taken: ";
            for (int index : path) {
                cout << arr[index] << " ";
            }
            cout << endl;
            return mid; // Found the target
        } 
        else if (arr[mid] < target) {
            low = mid + 1; // Move right
        } 
        else {
            high = mid - 1; // Move left
        }
    }

    // Print path even if target isn't found
    cout << "Path taken: ";
    for (int index : path) {
        cout << arr[index] << " ";
    }
    cout << endl;

    return -1; // Target not found
}

int main() {
    vector<int> arr = {
        15, 23, 29, 33, 37,
        41, 44, 49, 52, 54,
        60, 62, 68, 70, 75,
        85, 90, 95, 100, 110
    };

    int target = 110;
    int index = binarySearch(arr, target);

    if (index != -1) {
        cout << "Element " << target << " found at index " << index << endl;
    } else {
        cout << "Element not found" << endl;
    }

    return 0;
}
