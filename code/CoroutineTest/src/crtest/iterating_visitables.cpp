#include <cstdlib>

#include <boost/bind.hpp>
#include <boost/coroutine/coroutine.hpp>

using namespace std;

template<typename Data>
class Visitor
{
public:
  virtual ~Visitor() { }
  virtual bool visit(Data const & data) = 0;
};

template <typename Data>
class Visitable
{
public:
    virtual ~Visitable() {}
    virtual void performVisits(Visitor<Data> & visitor) = 0;
};

class Counter : public Visitable<int>
{
public:
    Counter(int start, int end)
        : start_(start), end_(end) {}
    void performVisits(Visitor<int> & visitor)
    {
        bool terminated = false;
        for (int current=start_; !terminated && current<=end_; ++current)
            terminated = visitor.visit(current);
    }
private:
    int start_;
    int end_;
};

class CounterVisitor : public Visitor<int>
{
public:
    bool visit(int const & data)
    {
        std::cerr << data << std::endl;
        return false; // not terminated
    }
};

/*
int main(void)
{
    Counter counter(1, 100);
    CounterVisitor visitor;
    counter.performVisits(visitor);
    return EXIT_SUCCESS;
}
*/

template<typename Data>
class Iterator
{
public:
    virtual ~Iterator() {}
    virtual bool isValid()=0;
    virtual void moveToNext()=0;
    virtual Data const & getData()=0;
};

template<typename Data>
class VisitableIterator : public Iterator<Data>
{
private:
    typedef boost::coroutines::coroutine<void()> coro_t;
    typedef coro_t::caller_type caller_t;
public:
    VisitableIterator(Visitable<Data> & visitable)
        : valid_(true), visitable_(visitable)
    {
        coro_ = coro_t(boost::bind(&VisitableIterator::visitCoro, this, _1));
    }
    inline bool isValid()
    {
        return valid_;
    }
    inline Data const & getData()
    {
        return visitor_.getData();
    }
    inline void moveToNext()
    {
        if(valid_)
            coro_();
    }
private:
    class InternalVisitor : public Visitor<Data>
    {
    public:
        InternalVisitor() {}
        inline bool visit(Data const & data)
        {
            data_ = &data;
            (*caller_)(); // return back to caller
            return false;
        }
        inline void setCaller(caller_t & caller)
        {
            caller_ = &caller;
        }
        inline Data const & getData()
        {
            return *data_;
        }
    private:
        Data const * data_;
        caller_t * caller_;
    };
    void visitCoro(coro_t::caller_type & caller)
    {
        visitor_.setCaller(caller);
        visitable_.performVisits(static_cast<Visitor<Data> &>(visitor_));
        valid_ = false;
    }
private:
    bool valid_;
    coro_t coro_;
    InternalVisitor visitor_;
    Visitable<Data> & visitable_;
};

int main(void)
{
    Counter counter(1, 100);
    VisitableIterator<int> iter(static_cast<Visitable<int>&>(counter));
    for (; iter.isValid(); iter.moveToNext()) {
        int data = iter.getData();
        cerr << data << endl;
    }

    return EXIT_SUCCESS;
}
