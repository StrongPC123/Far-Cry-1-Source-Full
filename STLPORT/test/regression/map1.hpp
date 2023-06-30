
// class Int is defined here because the primitive 'int' does not have a 
// default constructor. The default constructor is used when map['z']
// accesses an uninitialized element.
struct Int
  {
  Int(int x = 0) : val(x) {};
  int val;
  };

