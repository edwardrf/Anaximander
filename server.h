#include <boost/thread.hpp>

boost::thread* startServer(int portno, void task(int));