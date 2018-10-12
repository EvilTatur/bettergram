#pragma once

#include "cryptoprice.h"

#include <QObject>

namespace Bettergram {

/**
 * @brief The CryptoPriceList class contains list of CryptoPrice instances.
 */
class CryptoPriceList : public QObject {
	Q_OBJECT

public:
	enum class SortOrder {
		Rank,

		NameAscending,
		NameDescending,

		PriceAscending,
		PriceDescending,

		ChangeFor24hAscending,
		ChangeFor24hDescending,
	};

	typedef QList<QSharedPointer<CryptoPrice>>::const_iterator const_iterator;
	typedef QList<QSharedPointer<CryptoPrice>>::iterator iterator;

	explicit CryptoPriceList(QObject *parent = nullptr);

	std::optional<double> marketCap() const;
	const QString &marketCapString() const;
	void setMarketCap(const std::optional<double> &marketCap);

	std::optional<double> btcDominance() const;
	const QString &btcDominanceString() const;
	void setBtcDominance(const std::optional<double> &btcDominance);

	int freq() const;

	QDateTime lastUpdate() const;
	QString lastUpdateString() const;

	QSharedPointer<CryptoPrice> at(int index) const;
	int count() const;

	QList<QSharedPointer<CryptoPrice>> favoriteList() const;

	SortOrder sortOrder() const;
	void setSortOrder(const SortOrder &sortOrder);

	const QString &filterText() const;
	void setFilterText(const QString &filterText);

	bool isShowOnlyFavorites() const;
	void setIsShowOnlyFavorites(bool isShowOnlyFavorites);
	void toggleIsShowOnlyFavorites();

	const QString &sortString();
	const QString &orderString();

	bool areNamesFetched() const;

	QStringList getShortNames(int startIndex, int count) const;

	void parseNames(const QByteArray &byteArray);
	void parseValues(const QByteArray &byteArray, const QUrl &url, const QStringList &shortNames);
	void emptyValues();

	void save() const;
	void load();

	void createTestData();

public slots:

signals:
	void marketCapChanged();
	void btcDominanceChanged();
	void freqChanged();
	void sortOrderChanged();
	void filterTextChanged();
	void isShowOnlyFavoritesChanged();

	void namesUpdated();
	void valuesUpdated(const QUrl &url, const QList<QSharedPointer<CryptoPrice>> &prices);

protected:

private:
	/// Default frequency of updates in seconds
	static const int _defaultFreq;

	QList<QSharedPointer<CryptoPrice>> _list;
	QList<QSharedPointer<CryptoPrice>> _favoriteList;

	std::optional<double> _marketCap = std::nullopt;
	QString _marketCapString;

	std::optional<double> _btcDominance = std::nullopt;
	QString _btcDominanceString;

	/// Frequency of updates in seconds
	int _freq;

	QDateTime _lastUpdate;
	QString _lastUpdateString;

	SortOrder _sortOrder = SortOrder::Rank;
	QString _filterText;
	bool _isShowOnlyFavorites = false;

	bool _areNamesFetched = false;

	static const QString &getSortString(SortOrder sortOrder);
	static const QString &getOrderString(SortOrder sortOrder);

	static bool containsName(const QList<CryptoPrice> &priceList, const QString &name);

	static bool containsShortName(const QList<QSharedPointer<CryptoPrice>> &priceList,
								  const QString &shortName);

	static bool sortByRankAsc(const QSharedPointer<CryptoPrice> &price1,
							  const QSharedPointer<CryptoPrice> &price2);

	static bool sortByRankDesc(const QSharedPointer<CryptoPrice> &price1,
							   const QSharedPointer<CryptoPrice> &price2);

	static bool sortByNameAsc(const QSharedPointer<CryptoPrice> &price1,
							  const QSharedPointer<CryptoPrice> &price2);

	static bool sortByNameDesc(const QSharedPointer<CryptoPrice> &price1,
							   const QSharedPointer<CryptoPrice> &price2);

	static bool sortByPriceAsc(const QSharedPointer<CryptoPrice> &price1,
							   const QSharedPointer<CryptoPrice> &price2);

	static bool sortByPriceDesc(const QSharedPointer<CryptoPrice> &price1,
								const QSharedPointer<CryptoPrice> &price2);

	static bool sortBy24hAsc(const QSharedPointer<CryptoPrice> &price1,
							 const QSharedPointer<CryptoPrice> &price2);

	static bool sortBy24hDesc(const QSharedPointer<CryptoPrice> &price1,
							  const QSharedPointer<CryptoPrice> &price2);

	static bool sortByDoubleAsc(const QSharedPointer<CryptoPrice> &price1,
								const QSharedPointer<CryptoPrice> &price2,
								const std::optional<double> &value1,
								const std::optional<double> &value2);

	static bool sortByDoubleDesc(const QSharedPointer<CryptoPrice> &price1,
								 const QSharedPointer<CryptoPrice> &price2,
								 const std::optional<double> &value1,
								 const std::optional<double> &value2);

	void sort(QList<QSharedPointer<CryptoPrice>> &list, bool isFavoriteList);

	void setFreq(int freq);
	void setLastUpdate(const QDateTime &lastUpdate);
	void setAreNamesFetched(bool areNamesFetched);

	void updateMarketCapString();
	void updateBtcDominanceString();

	void updateFavoriteList();
	void sortFavoriteList();

	void addPrivate(const QSharedPointer<CryptoPrice> &price);

	QSharedPointer<CryptoPrice> find(const CryptoPrice *pricePointer);
	QSharedPointer<CryptoPrice> findByName(const QString &name, const QString &shortName);
	QSharedPointer<CryptoPrice> findByShortName(const QString &shortName);

	QList<QSharedPointer<CryptoPrice>> parsePriceListValues(const QJsonArray &priceListJson);

	/// If we ask server for prices by names it may return not all requested prices.
	/// To fix that we should fill missed prices
	void fillMissedPrices(QList<QSharedPointer<CryptoPrice>> &prices,
						  const QStringList &shortNames);

	void updateData(double marketCap, double btcDominance, int freq);
	void mergeCryptoPriceList(const QList<CryptoPrice> &priceList);

	void clear();

	void addTestData(const QUrl &url,
					 const QUrl &iconUrl,
					 const QString &name,
					 const QString &shortName,
					 int rank,
					 double currentPrice,
					 double changeFor24Hours,
					 CryptoPrice::Direction minuteDirection);

private slots:
	void onIconChanged();
	void onIsFavoriteToggled();
};

} // namespace Bettergram
