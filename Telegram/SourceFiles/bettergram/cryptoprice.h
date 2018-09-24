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
	static CryptoPrice *load(const QSettings &settings);

	explicit CryptoPrice(const QUrl &url,
						 const QUrl &iconUrl,
						 const QString &name,
						 const QString &shortName,
						 int rank,
						 QObject *parent = nullptr);

	explicit CryptoPrice(const QUrl &url,
						 const QUrl &iconUrl,
						 const QString &name,
						 const QString &shortName,
						 int rank,
						 double currentPrice,
						 double changeFor24Hours,
						 bool isCurrentPriceGrown,
						 QObject *parent = nullptr);

	explicit CryptoPrice(const CryptoPrice &price, QObject *parent = nullptr);

	CryptoPrice &operator=(const CryptoPrice &price);

	const QUrl &url() const;
	const QUrl &iconUrl() const;
	const QPixmap &icon() const;
	const QString &name() const;
	const QString &shortName() const;

	int rank() const;

	double currentPrice() const;
	QString currentPriceString() const;
	void setCurrentPrice(double currentPrice);

	double changeFor24Hours() const;
	QString changeFor24HoursString() const;
	void setChangeFor24Hours(double changeFor24Hours);

	bool isCurrentPriceGrown() const;
	void setIsCurrentPriceGrown(bool isCurrentPriceGrown);

	bool isChangeFor24HoursGrown() const;

	bool isEmpty() const;

	void updateData(const CryptoPrice &price);

	void save(QSettings &settings) const;

public slots:

signals:
	void iconChanged();
	void rankChanged();

	void currentPriceChanged();
	void changeFor24HoursChanged();

	void isCurrentPriceGrownChanged();
	void isChangeFor24HoursGrownChanged();

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
	double _currentPrice;

	/// Price change of the cryptocurrency for the latest 24 hours. For example: -3.22%
	double _changeFor24Hours;

	/// True if the current price of the cryptocurrency is grown
	bool _isCurrentPriceGrown;

	/// True if the price change of the cryptocurrency for the latest 24 hours is grown
	bool _isChangeFor24HoursGrown;

	void setUrl(const QUrl &url);
	void setIcon(const QSharedPointer<RemoteImage> &icon);
	void setName(const QString &name);
	void setShortName(const QString &shortName);
	void setRank(int rank);
	void setIsChangeFor24HoursGrown(bool isChangeFor24HoursGrown);
};

} // namespace Bettergram
