#include "storage-ring.h"

// cumbia
#include <cumbiapool.h>
#include <cuserviceprovider.h>
#include <cumacros.h>
#include <quapps.h>
// cumbia

#include <qustorageringview.h>
#include <QGridLayout>

Storagering::Storagering(CumbiaPool *cumbia_pool, QWidget *parent) :
    QWidget(parent)
{
    // cumbia
    CuModuleLoader mloader(cumbia_pool, &m_ctrl_factory_pool, &m_log_impl);
    cu_pool = cumbia_pool;

    // provide access to the engine in case of runtime swap
    new CuEngineAccessor(this, &cu_pool, &m_ctrl_factory_pool);

    QGridLayout *lo = new QGridLayout(this);
    QuStorageRingView *view = new QuStorageRingView(this, cu_pool, m_ctrl_factory_pool);
    lo->addWidget(view, 0, 0, 10, 10);
    bool ok = view->load();
    if(view->msgs().size() > 0) {
        for(int i = 0; i < view->msgs().size(); i++)
            perr("%s: load error: %s", qstoc(view->msgs()[i]));
    }

    // mloader.modules() to get the list of loaded modules
    // cumbia
}

Storagering::~Storagering() {
}
