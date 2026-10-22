#include "Customer.h"

Customer::Customer(int id,
                   const QString &name,
                   const QString &phone,
                   const QString &email,
                   const QString &serviceAddress)
    : m_id(id),
      m_name(name),
      m_phone(phone),
      m_email(email),
      m_serviceAddress(serviceAddress)
{
}

int Customer::id() const { return m_id; }
void Customer::setId(int id) { m_id = id; }

QString Customer::name() const { return m_name; }
void Customer::setName(const QString &name) { m_name = name; }

QString Customer::phone() const { return m_phone; }
void Customer::setPhone(const QString &phone) { m_phone = phone; }

QString Customer::email() const { return m_email; }
void Customer::setEmail(const QString &email) { m_email = email; }

QString Customer::serviceAddress() const { return m_serviceAddress; }
void Customer::setServiceAddress(const QString &serviceAddress) { m_serviceAddress = serviceAddress; }
