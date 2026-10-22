#pragma once

#include <QString>

class Technician {
public: 
	Technician()=default;
	Technician(int id, const QString &name, bool active);

	int id() const;
	void setId(int id);

	QString name() const;
	void setName(const QString &name);

	bool active() const;
	void setActive(bool active);

private:
	int m_id=0;
	QString m_name;
	bool m_active=true;
};