//Header file for the customer class.
#include "Customer.h"

//Constructor for the Customer class. Initializing a customer object with its database ID, name, ...
//...phone number, email address, and service address.
Customer::Customer(int id, const QString &name, const QString &phone,const QString &email, const QString &serviceAddress) : m_id(id), m_name(name), m_phone(phone),m_email(email), m_serviceAddress(serviceAddress){ }

//Getting and setting the customers unique ID in the database.
int Customer::id() const { return m_id; }
void Customer::setId(int id) { m_id = id; }

//Gettting and setting the customer's name
QString Customer::name() const { return m_name; }
void Customer::setName(const QString &name) { m_name = name; }

//Getting and setting the customers' phone number
QString Customer::phone() const { return m_phone; }
void Customer::setPhone(const QString &phone) { m_phone = phone; }

//Getting and setting the customers email address.
QString Customer::email() const { return m_email; }
void Customer::setEmail(const QString &email) { m_email = email; }

//Getting and setting the customer's service address.
QString Customer::serviceAddress() const { return m_serviceAddress; }
void Customer::setServiceAddress(const QString &serviceAddress) { m_serviceAddress = serviceAddress; }
