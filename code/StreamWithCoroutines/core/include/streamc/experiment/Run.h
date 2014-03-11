#ifndef STREAMC_EXPERIMENT_RUN_H
#define STREAMC_EXPERIMENT_RUN_H

#define BOOST_NO_CXX11_RVALUE_REFERENCES

#include <string>

namespace streamc { namespace experiment
{
    class Run {
    public:
        virtual ~Run() {}
        virtual void process() = 0;
        void run();
    protected:
        std::string getClassName();
    };

} } /* namespace */

#endif /* STREAMC_EXPERIMENT_RUN_H */
