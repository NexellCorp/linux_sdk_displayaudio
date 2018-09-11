#ifndef SHADOWEFFECT_H
#define SHADOWEFFECT_H

#include <QGraphicsDropShadowEffect>
#include <QGraphicsEffect>

class ShadowEffect : public QGraphicsEffect
{
	Q_OBJECT

public:
	explicit ShadowEffect(QObject *parent = 0);

	void draw(QPainter* painter);
	QRectF boundingRectFor(const QRectF& rect) const;

	inline void setDistance(qreal distance) { m_fDistance = distance; updateBoundingRect(); }
	inline qreal distance() const { return m_fDistance; }

	inline void setBlurRadius(qreal blurRadius) { m_fBlurRadius = blurRadius;
												  updateBoundingRect(); }
	inline qreal blurRadius() const { return m_fBlurRadius; }

	inline void setColor(const QColor& color) { m_Color = color; }
	inline QColor color() const { return m_Color; }

private:
	qreal  m_fDistance;
	qreal  m_fBlurRadius;
	QColor m_Color;
};

#endif // SHADOWEFFECT_H
