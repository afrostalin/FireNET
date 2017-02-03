#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QTextCodec>

QString m_root = "../";
QString m_bin_folder = m_root + "bin";

QString m_3rd_folder = m_root + "3rd/";
QString m_qt_folder = m_3rd_folder + "qt/";
QString m_qt_plugins = m_qt_folder + "plugins/";

QStringList m_ServerFileList = {
	"server_files/scripts/server_list.xml",
	"server_files/scripts/shop.xml",
	"server_files/settings/Debug/server.cfg",
	"server_files/settings/Release/server.cfg",
	"server_files/key.key",
	"server_files/key.pem"
};

QStringList m_WinFileList = { 
	m_qt_plugins + "platforms/qwindowsd.dll",
	m_qt_plugins + "platforms/qwindows.dll",
	m_qt_plugins + "sqldrivers/qsqlmysqld.dll",
	m_qt_plugins + "sqldrivers/qsqlmysql.dll",
	m_qt_folder + "bin/Qt5Cored.dll",
	m_qt_folder + "bin/Qt5Core.dll",
	m_qt_folder + "bin/Qt5Guid.dll",
	m_qt_folder + "bin/Qt5Gui.dll",
	m_qt_folder + "bin/Qt5Networkd.dll",
	m_qt_folder + "bin/Qt5Network.dll",
	m_qt_folder + "bin/Qt5Sqld.dll",
	m_qt_folder + "bin/Qt5Sql.dll",
	m_qt_folder + "bin/Qt5Widgetsd.dll",
	m_qt_folder + "bin/Qt5Widgets.dll",
	m_3rd_folder + "cutelogger/bin/Debug/Logger.dll",
	m_3rd_folder + "cutelogger/bin/Release/Logger.dll",
	m_3rd_folder + "openssl/bin/libeay32.dll",
	m_3rd_folder + "openssl/bin/ssleay32.dll",
	m_3rd_folder + "mysql/bin/libmySQL.dll"
};

bool DeployWindows(const QString &buildType)
{
	if (buildType == "Debug")
	{
		QDir().mkdir(m_bin_folder);
		QDir().mkdir(m_bin_folder + "/Windows");
		QDir().mkdir(m_bin_folder + "/Windows/server");
		QDir().mkdir(m_bin_folder + "/Windows/server/Debug");
		QDir().mkdir(m_bin_folder + "/Windows/server/Debug/plugins");
		QDir().mkdir(m_bin_folder + "/Windows/server/Debug/plugins/sqldrivers");
		QDir().mkdir(m_bin_folder + "/Windows/server/Debug/platforms");
		QDir().mkdir(m_bin_folder + "/Windows/server/Debug/scripts");

		m_bin_folder += "/Windows/server/Debug";

		qInfo() << "Step 1 - Check server files exists :";
		for (int i = 0; i < m_ServerFileList.size(); i++)
		{
			QString fileName = m_ServerFileList[i];
			if (!fileName.isEmpty())
			{
				if (QFile::exists(fileName))
				{
					qInfo() << fileName << "finded...";
				}
				else
				{
					qWarning() << "ERROR : File" << fileName << "not found";
					return false;
				}
			}
			else
			{
				qWarning() << "ERROR : Can't check files exists - wrong file list";
				return false;
			}
		}

		qInfo() << "Step 2 - Check Windows files exists :";
		for (int i = 0; i < m_WinFileList.size(); i++)
		{
			QString fileName = m_WinFileList[i];
			if (!fileName.isEmpty())
			{
				if (QFile::exists(fileName))
				{
					qInfo() << fileName << "finded...";
				}
				else
				{
					qWarning() << "ERROR : File" << fileName << "not found";
					return false;
				}
			}
			else
			{
				qWarning() << "ERROR : Can't check files exists - wrong file list";
				return false;
			}
		}

		qInfo() << "Step 3 - Copying server files to bin directory :";
		for (int i = 0; i < m_ServerFileList.size(); i++)
		{
			QString fileName = m_ServerFileList[i];
			QString path;

			if (fileName.contains("server_list.xml"))
				path = m_bin_folder + "/scripts/server_list.xml";
			else if (fileName.contains("shop.xml"))
				path = m_bin_folder + "/scripts/shop.xml";
			else if (fileName.contains("Debug/server.cfg"))
				path = m_bin_folder + "/server.cfg";
			else if (fileName.contains("key.key"))
				path = m_bin_folder + "/key.key";
			else if (fileName.contains("key.pem"))
				path = m_bin_folder + "/key.pem";

			if (!path.isEmpty())
				qInfo() << "Copying" << fileName << "to" << path << QFile::copy(fileName, path);
		}
		
		qInfo() << "Step 4 - Copying Windows files to bin directory :";
		for (int i = 0; i < m_WinFileList.size(); i++)
		{
			QString fileName = m_WinFileList[i];
			QString path;

			if (fileName.contains("qwindowsd.dll"))
				path = m_bin_folder + "/platforms/qwindowsd.dll";
			else if (fileName.contains("qsqlmysqld.dll"))
				path = m_bin_folder + "/plugins/sqldrivers/qsqlmysqld.dll";
			else if (fileName.contains("Qt5Cored.dll"))
				path = m_bin_folder + "/Qt5Cored.dll";
			else if (fileName.contains("Qt5Guid.dll"))
				path = m_bin_folder + "/Qt5Guid.dll";
			else if (fileName.contains("Qt5Networkd.dll"))
				path = m_bin_folder + "/Qt5Networkd.dll";
			else if (fileName.contains("Qt5Sqld.dll"))
				path = m_bin_folder + "/Qt5Sqld.dll";
			else if (fileName.contains("Qt5Widgetsd.dll"))
				path = m_bin_folder + "/Qt5Widgetsd.dll";
			else if (fileName.contains("Debug/Logger.dll"))
				path = m_bin_folder + "/Logger.dll";
			else if (fileName.contains("libeay32.dll"))
				path = m_bin_folder + "/libeay32.dll";
			else if (fileName.contains("ssleay32.dll"))
				path = m_bin_folder + "/ssleay32.dll";
			else if (fileName.contains("libmySQL.dll"))
				path = m_bin_folder + "/libmySQL.dll";

			if (!path.isEmpty())
				qInfo() << "Copying" << fileName << "to" << path << QFile::copy(fileName, path);
		}

		return true;
	}
	else if (buildType == "Release")
	{
		QDir().mkdir(m_bin_folder);
		QDir().mkdir(m_bin_folder + "/Windows");
		QDir().mkdir(m_bin_folder + "/Windows/server");
		QDir().mkdir(m_bin_folder + "/Windows/server/Release");
		QDir().mkdir(m_bin_folder + "/Windows/server/Release/plugins");
		QDir().mkdir(m_bin_folder + "/Windows/server/Release/plugins/sqldrivers");
		QDir().mkdir(m_bin_folder + "/Windows/server/Release/platforms");
		QDir().mkdir(m_bin_folder + "/Windows/server/Release/scripts");

		m_bin_folder += "/Windows/server/Release";

		qInfo() << "Step 1 - Check server files exists :";
		for (int i = 0; i < m_ServerFileList.size(); i++)
		{
			QString fileName = m_ServerFileList[i];
			if (!fileName.isEmpty())
			{
				if (QFile::exists(fileName))
				{
					qInfo() << fileName << "finded...";
				}
				else
				{
					qWarning() << "ERROR : File" << fileName << "not found";
					return false;
				}
			}
			else
			{
				qWarning() << "ERROR : Can't check files exists - wrong file list";
				return false;
			}
		}

		qInfo() << "Step 2 - Check Windows files exists :";
		for (int i = 0; i < m_WinFileList.size(); i++)
		{
			QString fileName = m_WinFileList[i];
			if (!fileName.isEmpty())
			{
				if (QFile::exists(fileName))
				{
					qInfo() << fileName << "finded...";
				}
				else
				{
					qWarning() << "ERROR : File" << fileName << "not found";
					return false;
				}
			}
			else
			{
				qWarning() << "ERROR : Can't check files exists - wrong file list";
				return false;
			}
		}

		qInfo() << "Step 3 - Copying server files to bin directory :";
		for (int i = 0; i < m_ServerFileList.size(); i++)
		{
			QString fileName = m_ServerFileList[i];
			QString path;

			if (fileName.contains("server_list.xml"))
				path = m_bin_folder + "/scripts/server_list.xml";
			else if (fileName.contains("shop.xml"))
				path = m_bin_folder + "/scripts/shop.xml";
			else if (fileName.contains("Release/server.cfg"))
				path = m_bin_folder + "/server.cfg";
			else if (fileName.contains("key.key"))
				path = m_bin_folder + "/key.key";
			else if (fileName.contains("key.pem"))
				path = m_bin_folder + "/key.pem";

			if (!path.isEmpty())
				qInfo() << "Copying" << fileName << "to" << path << QFile::copy(fileName, path);
		}

		qInfo() << "Step 4 - Copying Windows files to bin directory :";
		for (int i = 0; i < m_WinFileList.size(); i++)
		{
			QString fileName = m_WinFileList[i];
			QString path;

			if (fileName.contains("qwindows.dll"))
				path = m_bin_folder + "/platforms/qwindows.dll";
			else if (fileName.contains("qsqlmysql.dll"))
				path = m_bin_folder + "/plugins/sqldrivers/qsqlmysql.dll";
			else if (fileName.contains("Qt5Core.dll"))
				path = m_bin_folder + "/Qt5Core.dll";
			else if (fileName.contains("Qt5Gui.dll"))
				path = m_bin_folder + "/Qt5Gui.dll";
			else if (fileName.contains("Qt5Network.dll"))
				path = m_bin_folder + "/Qt5Network.dll";
			else if (fileName.contains("Qt5Sql.dll"))
				path = m_bin_folder + "/Qt5Sql.dll";
			else if (fileName.contains("Qt5Widgets.dll"))
				path = m_bin_folder + "/Qt5Widgets.dll";
			else if (fileName.contains("Release/Logger.dll"))
				path = m_bin_folder + "/Logger.dll";
			else if (fileName.contains("libeay32.dll"))
				path = m_bin_folder + "/libeay32.dll";
			else if (fileName.contains("ssleay32.dll"))
				path = m_bin_folder + "/ssleay32.dll";
			else if (fileName.contains("libmySQL.dll"))
				path = m_bin_folder + "/libmySQL.dll";

			if (!path.isEmpty())
				qInfo() << "Copying" << fileName << "to" << path << QFile::copy(fileName, path);
		}

		return true;
	}
	else
		qWarning() << "ERROR : Can't deploy - Unknown build type";

	return false;
}

bool DeployUnix(const QString &buildType)
{
	if (buildType == "Debug")
	{
		m_bin_folder += buildType;
	}
	else if (buildType == "Release")
	{
		m_bin_folder += buildType;
	}
	else
		qWarning() << "ERROR : Can't deploy - Unknown build type";

	return false;
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

	QString system_name;
	QString build_type;

	QTextStream stream(stdin);
	QString line;

	qInfo() << "Choose system [win/unix] :";
	line = stream.readLine();

	if (!line.isEmpty())
	{
		if (line.contains("win"))
			system_name = "Windows";
		else if (line.contains("unix"))
			system_name = "Unix";
		else
			qWarning() << "ERROR : Wrong system name";

		qInfo() << "Choose build type [debug/release] :";
		line = stream.readLine();

		if (!line.isEmpty())
		{
			if (line.contains("debug"))
				build_type = "Debug";
			else if (line.contains("release"))
				build_type = "Release";
			else
				qWarning() << "ERROR : Wrong build type";

			if (system_name == "Windows" && build_type == "Debug")
			{
				qInfo() << "Start deploying for Windows:Debug ...";
				if (DeployWindows(build_type))
					qInfo() << "---DEPLOYMENT COMPLEATE---";
			}
			else if (system_name == "Windows" && build_type == "Release")
			{
				qInfo() << "Start deploying for Windows:Release ...";
				if (DeployWindows(build_type))
					qInfo() << "---DEPLOYMENT COMPLEATE---";
			}
			else if (system_name == "Unix" && build_type == "Debug")
			{
				qInfo() << "Start deploying for Unix:Debug ...";
				DeployUnix(build_type);
			}
			else if (system_name == "Unix" && build_type == "Release")
			{
				qInfo() << "Start deploying for Unix:Release ...";
				DeployUnix(build_type);
			}
			else
				qWarning() << "ERROR : Unknown system name and build type";
		}
		else
			qWarning() << "ERROR : Empty parameters";
	}
	else
	{
		qWarning() << "ERROR : Empty parameters";
	}

    return a.exec();
}
