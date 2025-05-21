#include <iostream>
using namespace std;

typedef struct {
  int *arr;
  int size;
  int front;
  int back;
  int count;
} circular_buf;

void put(circular_buf &buf, int value) {
  buf.arr[buf.back] = value;
  buf.back = (buf.back + 1) % buf.size;

  if (buf.count == buf.size) {
    buf.front = (buf.front + 1) % buf.size;
  } else {
    buf.count++;
  }
}
int pull(circular_buf &buf, int &value) {
  if (buf.count == 0) {
    return -1;
  }
  buf.count--;
  value = buf.arr[buf.front];
  buf.front++;
  buf.front %= buf.size;
  return 0;
}

void print(circular_buf &buf) {
  for (int i = 0; i < buf.size; i++) {
    cout << buf.arr[i] << " ";
  }
  cout << "-f: " << buf.front << " -b: " << buf.back << " -s: " << buf.size
       << " -c: " << buf.count << endl;
}

int main() {
  circular_buf buf;
  buf.arr = new int[5];
  buf.size = 5;
  buf.front = 0;
  buf.back = 0;
  buf.count = 0;

  while (true) {
    char inp;
    cout << ">> ";
    cin >> inp;

    switch (inp) {
    case 'i': {
      int num;
      cin >> num;
      put(buf, num);
      break;
    }
    case 'o': {
      int num;
      if (pull(buf, num) == 0) {
        cout << " -- " << num << endl;
      } else {
        cout << "empty" << endl;
      }
      break;
    }
    }

    cout << endl;
    print(buf);
  }

  return 0;
}
