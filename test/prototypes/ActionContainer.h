#ifndef TEST_PROTOTYPES_ACTIONCONTAINER_H_
#define TEST_PROTOTYPES_ACTIONCONTAINER_H_

#include <fsfw/action/HasActionsIF.h>
#include <fsfw/controller/ControllerBase.h>
#include <type_traits>

template<class T, typename = void>
class Controller: public T {
public:
    void performActionContainerAction() {
        sif::info << "nothing!" << std::endl;
    };
};

template<class T>
class Controller<T,
    typename std::enable_if<std::is_base_of<HasActionsIF, T>::value>::type>:
    public T {
public:
    void performActionContainerAction() {
        sif::info << "action helper!" << std::endl;
    };
private:
    ActionHelper actionHelper;

};




#endif /* TEST_PROTOTYPES_ACTIONCONTAINER_H_ */
