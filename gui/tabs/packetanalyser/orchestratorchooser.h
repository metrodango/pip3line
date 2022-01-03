#ifndef ORCHESTRATORCHOOSER_H
#define ORCHESTRATORCHOOSER_H

#include <QWidget>
#include <QComboBox>

class SourcesOrchestatorAbstract;
class GuiHelper;
class DefaultDialog;

class OrchestratorChooser : public QComboBox
{
        Q_OBJECT
    public:
        explicit OrchestratorChooser(GuiHelper *guiHelper, SourcesOrchestatorAbstract *orchestrator = nullptr, QWidget *parent = nullptr);
        ~OrchestratorChooser();
        void setType(int type);
        SourcesOrchestatorAbstract *getOrchestrator();
        int showConfPanel(SourcesOrchestatorAbstract *targetOrchestrator, bool blocking = false);
    signals:
        void newOrchestrator(SourcesOrchestatorAbstract * orchestrator);
    public slots:
        void onGuiRequested();
    private slots:
        void onOrchestratorDeleted();
        void onSelection(int index);
    private:
        Q_DISABLE_COPY(OrchestratorChooser)
        SourcesOrchestatorAbstract *createOrchestratorFromType(int type);
        SourcesOrchestatorAbstract *orchestrator{nullptr};
        GuiHelper *guiHelper;
        QMetaObject::Connection destructorLink;
        DefaultDialog *confDialog;
};

#endif // ORCHESTRATORCHOOSER_H
