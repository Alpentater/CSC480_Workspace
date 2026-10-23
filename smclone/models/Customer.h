#pragma once

#include <QString>

class Customer {
public:
	Customer() = default;
	Customer(int id, const QString &name, const QString &phone, const QString &email, const QString &serviceAddress);

	int id() const;
	void setId(int id);

	QString name() const;
	void setName(const QString &name);

	QString phone() const;
	void setPhone(const QString &phone);

	QString email() const;
	void setEmail(const QString &email);

	QString serviceAddress() const;
	void setServiceAddress(const QString &serviceAddress);

private:
	int m_id = 0;
	QString m_name;	
	QString	m_phone;
	QString	m_email;
	QString	m_serviceAddress;
};