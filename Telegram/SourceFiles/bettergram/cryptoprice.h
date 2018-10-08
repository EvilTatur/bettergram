#pragma once

#include <QObject>

namespace Bettergram {

class RemoteImage;

/**
 * @brief The CryptoPrice class contains current price of one cryptocurrency.
 * See also https://www.livecoinwatch.com
 */
class CryptoPrice : public QObject {
	Q_OBJECT

public:
	enum class Direction {
		None,
		Up,
		Down
	};

	static QSharedPointer<CryptoPrice> load(const QSettings &settings);
	static Direction countDirection(double value);

	explicit CryptoPrice(const QUrl &url,
						 const QUrl &iconUrl,
						 const QString &name,
						 const QString &shortName,
						 bool isNeedDownloadIcon,
						 QObject *parent = nullptr);

	explicit CryptoPrice(const QUrl &url,
						 const QUrl &iconUrl,
						 const QString &name,
						 const QString &shortName,
						 int rank,
						 double currentPrice,
						 double changeFor24Hours,
						 Direction minuteDirection,
						 bool isNeedDownloadIcon,
						 QObject *parent = nullptr);

	explicit CryptoPrice(const CryptoPrice &price, QObject *parent = nullptr);

	CryptoPrice &operator=(const CryptoPrice &price);

	const QUrl &url() const;
	const QUrl &iconUrl() const;
	const QPixmap &icon() const;
	const QString &name() const;
	const QString &shortName() const;

	int rank() const;
	void setRank(int rank);

	double currentPrice() const;
	QString currentPriceString() const;
	void setCurrentPrice(double currentPrice);

	double changeFor24Hours() const;
	QString changeFor24HoursString() const;
	void setChangeFor24Hours(double changeFor24Hours);

	Direction minuteDirection() const;
	void setMinuteDirection(Direction minuteDirection);

	Direction dayDirection() const;

	bool isEmpty() const;

	void updateData(const CryptoPrice &price);
	void resetValues();

	void downloadIconIfNeeded();
	void forceDownloadIcon();

	void save(QSettings &settings) const;

public slots:

signals:
	void iconChanged();
	void rankChanged();

	void currentPriceChanged();
	void changeFor24HoursChanged();

	void minuteDirectionChanged();
	void dayDirectionChanged();

protected:

private:
	/// Site address of the information about the cryptocurrency. For example: https://www.livecoinwatch.com/price/Bitcoin-BTC
	QUrl _url;

	/// Cryptocurrency icon. For example: https://www.livecoinwatch.com/images/icons32/btc.png
	QSharedPointer<RemoteImage> _icon;

	/// Name of the cryptocurrency. For example: Bitcoin
	QString _name;

	/// Short name of the cryptocurrency. For example: BTC
	QString _shortName;

	/// Rank of the cryptocurrency
	int _rank = 0;

	/// Current price of the cryptocurrency. For example: $7935.96
	double _currentPrice = std::numeric_limits<double>::quiet_NaN();
	QString _currentPriceString;

	/// Price change of the cryptocurrency for the latest 24 hours. For example: -3.22%
	double _changeFor24Hours = std::numeric_limits<double>::quiet_NaN();
	QString _changeFor24HoursString;

	Direction _minuteDirection = Direction::None;
	Direction _dayDirection = Direction::None;

	void setUrl(const QUrl &url);
	void setIcon(const QSharedPointer<RemoteImage> &icon);
	void setName(const QString &name);
	void setShortName(const QString &shortName);
	void setDayDirection(Direction dayDirection);

	void updateCurrentPriceString();
	void updateChangeFor24HoursString();

	QString iconFilePath() const;

	void loadIcon();
	void saveIcon() const;
};

} // namespace Bettergram
