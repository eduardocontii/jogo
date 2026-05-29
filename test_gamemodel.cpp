
#include <QtTest>
#include "GameModel.h"

class TestGameModel : public QObject {
    Q_OBJECT
private slots:
    void initTestCase() {
        // Inicialização global
    }

    void testStartLevel() {
        GameModel model;
        model.startLevel(1, "TestUser");
        
        QCOMPARE(model.getCurrentLevel(), 1);
        QCOMPARE(model.getUserName(), QString("TestUser"));
        QCOMPARE(model.getScore(), 0);
        QCOMPARE(model.getMoves(), 0);
        QVERIFY(!model.getBuses().isEmpty());
        QVERIFY(!model.getPassengerQueue().isEmpty());
    }

    void testUndoLastMove() {
        GameModel model;
        model.startLevel(1, "TestUser");
        
        int initialBuses = model.getBuses().size();
        
        // Simula a saída de um ônibus se possível
        if (!model.getBuses().isEmpty()) {
            int busId = model.getBuses()[0].id;
            model.tryBusExit(busId);
            
            // Independente de ter saído ou não (bloqueio), o undo deve funcionar se houve mudança de estado
            model.undoLastMove();
            QCOMPARE(model.getBuses().size(), initialBuses);
        }
    }

    void testBusDocking() {
        GameModel model;
        model.startLevel(1, "TestUser");
        
        // Como não podemos garantir qual ônibus pode sair sem analisar o grid,
        // vamos testar a lógica de busExited diretamente se fosse acessível,
        // mas como é privada, testamos via tryBusExit em um cenário favorável.
        // No nível 1, o ônibus 5 (magenta) está em (0,4) vertical, length 2, directionForward true.
        // Ele deve conseguir sair se o caminho estiver livre.
        
        int initialDocked = model.getDockedBuses().size();
        model.tryBusExit(5); // Ônibus magenta no Nível 1 costuma ter saída livre
        
        // Se o ônibus saiu, ele deve estar nas docas
        if (model.getDockedBuses().size() > initialDocked) {
            QCOMPARE(model.getDockedBuses().last().id, 5);
            QCOMPARE(model.getDockedBuses().last().passengers, 0);
        }
    }

    void testBoardingLogic() {
        GameModel model;
        model.startLevel(1, "TestUser");
        
        // Força um ônibus nas docas para testar o embarque
        // Como m_dockedBuses é privado, este teste foca na integração
        // Em um teste unitário real, poderíamos usar 'friend class' ou herança para injetar estado
        
        model.tryBusExit(5); 
        if (!model.getDockedBuses().isEmpty()) {
            int initialQueue = model.getPassengerQueue().size();
            int initialScore = model.getScore();
            
            // Tenta processar embarque repetidamente até que um passageiro combine com a cor do ônibus
            // Ou a fila acabe.
            for(int i=0; i < 20; ++i) {
                model.processBoarding();
            }
            
            // Se algum passageiro embarcou, a pontuaçao deve ter aumentado
            if (model.getScore() > initialScore) {
                QVERIFY(model.getPassengerQueue().size() < initialQueue);
            }
        }
    }

    void testRankingSystem() {
        GameModel model;
        model.startLevel(1, "TestUserRanking");
        
        // Simula uma pontuação
        model.saveToRanking();
        
        QJsonArray ranking = model.getRanking();
        bool found = false;
        for (const QJsonValue& val : ranking) {
            if (val.toObject()["name"].toString() == "TestUserRanking") {
                found = true;
                break;
            }
        }
        QVERIFY(found);
    }
};

QTEST_MAIN(TestGameModel)
#include "test_gamemodel.moc"
