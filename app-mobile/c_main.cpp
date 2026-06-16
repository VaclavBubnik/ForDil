/*
    Copyright 2025 - Present MENDELU

    This file is part of ForDil.

    ForDil is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    ForDil is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with ForDil. If not, see <https://www.gnu.org/licenses/>.
*/

#include <QApplication>
#include <QGuiApplication>
#include <QStyleHints>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QLoggingCategory>
#include "c_backend.h"

#ifdef Q_OS_ANDROID
#include <jni.h>
#include <QtCore/QJniObject>
#include <QtCore/QJniEnvironment>
#endif

int main(int argc, char *argv[])
{
    // Create the application object
    QApplication app(argc, argv);

    QGuiApplication::styleHints()->setColorScheme(Qt::ColorScheme::Light);

#ifdef QT_DEBUG
    QLoggingCategory::setFilterRules("*.debug=true");
#else
    QLoggingCategory::setFilterRules(
        "*.debug=false\n"
        "*.info=false\n"
        "*.warning=true\n"
        "*.critical=true"
    );
#endif

    // Create the QML application engine
    QQmlApplicationEngine engine;

    // Instantiate the backend and pass the application object to it
    Backend backend(&app);
    backend.init();

    // Register the Backend class as a QML type for usage in QML
    qmlRegisterType<Backend>("ForDil", 1, 0, "Backend");

    // Register a singleton instance of the Backend class for QML
    if (qmlRegisterSingletonInstance("ForDil", 1, 0, "Backend", &backend))
    {
        qDebug() << "Application C++ backend registered successfully";
    }
    else
    {
        qWarning() << "Application C++ backend registration failed!";
    }

    // Set the language for the application
    backend.setLanguage();

    // Setup application window
    backend.setupWindow();

    // Add an image provider for the QML engine
    engine.addImageProvider("fordilimageprovider", backend.getImageProvider());

    // Build date to QML
    engine.rootContext()->setContextProperty("buildDate", QString(APP_BUILD_DATE));

    // Connect the engine's object creation failure signal to exit the application
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); }, // Exit the application with an error code
        Qt::QueuedConnection);

    // Load the main QML module and page (PageMain)
    engine.loadFromModule("ForDil", "PageMain"); // Qt 6.5+
    //engine.load(QUrl(u"qrc:/ForDil/PageMain.qml"_qs)); // Qt 6.4

    // Check if no root objects were created successfully
    if (engine.rootObjects().isEmpty())
    {
        return -1; // Exit with an error code
    }

    // Start the application event loop
    return app.exec();
}
