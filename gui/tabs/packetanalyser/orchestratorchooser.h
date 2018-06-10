#ifndef ORCHESTRATORCHOOSER_H
#define ORCHESTRATORCHOOSER_H

#include <QWidget>
#include <QComboBox>

class SourcesOrchestatorAbstract;
class GuiHelper;

class OrchestratorChooser : public QComboBox
{
        Q_OBJECT
    public:
        explicit OrchestratorChooser(GuiHelper *guiHelper, SourcesOrchestatorAbstract *orchestrator = nullptr, QWidget *parent = nullptr);
        ~OrchestratorChooser();
        void setType(int type);
        SourcesOrchestatorAbstract *getOrchestrator();
    signals:
        void newOrchestrator(SourcesOrchestatorAbstract * orchestrator);
    public slots:
        int showConfPanel(bool blocking = false);
    private slots:
        void onOrchestratorDeleted();
        void onSelection(int index);
    private:
        SourcesOrchestatorAbstract *createOrchestratorFromType(int type);
        SourcesOrchestatorAbstract *orchestrator{nullptr};
        GuiHelper *guiHelper;
        QMetaObject::Connection destructorLink;
};

#endif // ORCHESTRATORCHOOSER_H
