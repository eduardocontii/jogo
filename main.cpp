#include "busstopwindows.h" //importa o esqueleto, sem ele o main nao sabe oque e busstopwindows

#include <QApplication> // e o motor do QT, gerencia o mouse o teclado, o qt precisa dele
#include <QLocale>      //celular detecta em qual idioma esta o celular e converte
#include <QTranslator>

int main(
    int argc,
    char *argv[]) // São argumentos que o sistema operacional passa para o programa (como abrir um arquivo específico ao dar dois cliques nele).
{
    QApplication
        a(argc,
          argv); //a, é o coração do programa. Ele controla o fluxo de eventos (cliques, toques na tela, etc).

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "BusStop_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }
    BusStopWindows w; // cria  a janela na memoria do computador
    w.show();         // por padrao as janelas em c++ nesce invisivel, esse comando faz ela aparecer
    return QCoreApplication::
        exec(); // essa linha coloca o programa em loop infinito, esperando dar um clique na telaaa
}
