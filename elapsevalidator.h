#ifndef ELAPSEVALIDATOR_H
#define ELAPSEVALIDATOR_H

#include <QDebug>
#include <QObject>
#include <functional>
#include <atomic>

class AbstrElapseValidator
{
public:
    virtual void addElements(long elmnts) = 0;
    virtual bool aborted() const = 0;
};

class ElapseValidator : public AbstrElapseValidator
{
public:
    explicit ElapseValidator();
    ~ElapseValidator();

    void addElements(long elmnts) override;

    bool aborted() const override;

    bool limitReached() const ;

    bool cancelled() const;
    void setCancelled();

    void setMaximumCounter(int maxCoutr);
private:
    long m_maxCounter;
    std::function<long(int)> m_incrementor;
    std::atomic<long> m_counter;
    std::atomic<bool> m_cancelled;
    std::atomic<bool> m_limitReached;
};

#endif // ELAPSEVALIDATOR_H
