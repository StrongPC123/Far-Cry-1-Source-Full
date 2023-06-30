class X
{
  public:
    X(int i_ = 0) : i(i_) {}
    ~X() {}
    operator int() const { return i; }
 
  private:
    int i;
};

