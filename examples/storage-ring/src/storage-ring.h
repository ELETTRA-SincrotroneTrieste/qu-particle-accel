#ifndef Storagering_H
#define Storagering_H

#include <QWidget>

// cumbia
#include <qulogimpl.h>
#include <cucontrolsfactorypool.h>
class CumbiaPool;
// cumbia

namespace Ui {
class Storagering;
}

class Storagering : public QWidget
{
    Q_OBJECT

public:
    explicit Storagering(CumbiaPool *cu_p, QWidget *parent = 0);
    ~Storagering();

private:
    Ui::Storagering *ui;

    // cumbia
    CumbiaPool *cu_pool;
    QuLogImpl m_log_impl;
    CuControlsFactoryPool m_ctrl_factory_pool;
    // cumbia
};

#endif // Storagering_H
