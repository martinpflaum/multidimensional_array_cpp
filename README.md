# multidimensional_array_cpp
A light weight implementation of an multidimensional array in cpp

# Usage
I tried to make it look very much like numpy arrays
```cpp
Array<int> a = {1,2,24,2};
Array<int> b = a + 9;
Array<int> c = a[{true,false,false,true}];
Array<int> x = a[{0,1,2,0,2,2,1,0}];
Array<int> a = {{1,2,3},{4,5,6}};
Array<int> b = a[_][{1,2}];
```
Some functionality is still missing like assigning a array to a sup part of the array,
also using [_][y] operators will be a bit confusing sometimes since it is implecitly (array[:])[y] in python
not standard [:,y].

Appart from that its pretty cool and you can implement algorithms fast with it in a python style syntax :)
