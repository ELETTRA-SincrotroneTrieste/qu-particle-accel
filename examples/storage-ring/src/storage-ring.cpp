#include "storage-ring.h"

// cumbia
#include <cumbiapool.h>
#include <cuserviceprovider.h>
#include <cumacros.h>
#include <quapps.h>
// cumbia

#include <qujson2svg_w.h>
#include <QGridLayout>
#include <QMessageBox>

Storagering::Storagering(CumbiaPool *cumbia_pool, QWidget *parent) :
    QWidget(parent)
{
    // cumbia
    CuModuleLoader mloader(cumbia_pool, &m_ctrl_factory_pool, &m_log_impl);
    cu_pool = cumbia_pool;

    // provide access to the engine in case of runtime swap
    new CuEngineAccessor(this, &cu_pool, &m_ctrl_factory_pool);

    QGridLayout *lo = new QGridLayout(this);
    QuJsonToSvgW *view = new QuJsonToSvgW(this);
    lo->addWidget(view, 0, 0, 10, 10);
    connect(view, SIGNAL(op(QStringList)), this, SLOT(onOperation(QStringList)));

    // mloader.modules() to get the list of loaded modules
    // cumbia
    resize(800, 600);
}

Storagering::~Storagering() {
}

void Storagering::onOperation(const QStringList &errors) {
    foreach(const QString& err, errors)
        perr("%s", qstoc(err));
}
