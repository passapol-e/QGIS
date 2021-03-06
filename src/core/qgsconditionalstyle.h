#ifndef QGSCONDITIONALSTYLE_H
#define QGSCONDITIONALSTYLE_H

#include "qgis_core.h"
#include <QFont>
#include <QColor>
#include <QPixmap>
#include <QDomNode>
#include <QDomDocument>

#include "qgssymbol.h"

class QgsConditionalStyle;
class QgsReadWriteContext;

typedef QList<QgsConditionalStyle> QgsConditionalStyles;


/** \ingroup core
 * \brief The QgsConditionalLayerStyles class holds conditional style information
 * for a layer. This includes field styles and full row styles.
 */
class CORE_EXPORT QgsConditionalLayerStyles
{
  public:
    QgsConditionalLayerStyles();

    QList<QgsConditionalStyle> rowStyles();

    /**
     * \brief Set the conditional styles that apply to full rows of data in the attribute table.
     * Each row will check be checked against each rule.
     * \param styles The styles to assign to all the rows
     * \since QGIS 2.12
     */
    void setRowStyles( const QList<QgsConditionalStyle> &styles );

    /**
     * \brief Set the conditional styles for the field UI properties.
     * \param fieldName name of field
     * \param styles
     */
    void setFieldStyles( const QString &fieldName, const QList<QgsConditionalStyle> &styles );

    /**
     * \brief Returns the conditional styles set for the field UI properties
     * \returns A list of conditional styles that have been set.
     */
    QList<QgsConditionalStyle> fieldStyles( const QString &fieldName );

    /** Reads field ui properties specific state from Dom node.
     */
    bool readXml( const QDomNode &node, const QgsReadWriteContext &context );

    /** Write field ui properties specific state from Dom node.
     */
    bool writeXml( QDomNode &node, QDomDocument &doc, const QgsReadWriteContext &context ) const;

  private:
    QHash<QString, QgsConditionalStyles> mFieldStyles;
    QList<QgsConditionalStyle> mRowStyles;
};

/** \class QgsConditionalStyle
 *  \ingroup core
 * Conditional styling for a rule.
 */
class CORE_EXPORT QgsConditionalStyle
{
  public:
    QgsConditionalStyle();
    QgsConditionalStyle( const QgsConditionalStyle &other );
    QgsConditionalStyle( const QString &rule );

    QgsConditionalStyle &operator=( const QgsConditionalStyle &other );

    /**
     * \brief Check if the rule matches using the given value and feature
     * \param value The current value being checked. The "value" variable from the context is replaced with this value.
     * \param context Expression context for evaluating rule expression
     * \returns True of the rule matches against the given feature
     */
    bool matches( const QVariant &value, QgsExpressionContext &context ) const;

    /**
     * \brief Render a preview icon of the rule.
     * \returns QPixmap preview of the style
     */
    QPixmap renderPreview() const;

    /**
     * \brief Set the name of the style.  Names are optional but handy for display
     * \param value The name given to the style
     */
    void setName( const QString &value ) { mName = value; mValid = true; }

    /**
     * \brief Set the rule for the style.  Rules should be of QgsExpression syntax.
     * Special value of \@value is replaced at run time with the check value
     * \param value The QgsExpression style rule to use for this style
     */
    void setRule( const QString &value ) { mRule = value; mValid = true; }

    /**
     * \brief Set the background color for the style
     * \param value QColor for background color
     */
    void setBackgroundColor( const QColor &value ) { mBackColor = value; mValid = true; }

    /**
     * \brief Set the text color for the style
     * \param value QColor for text color
     */
    void setTextColor( const QColor &value ) { mTextColor = value; mValid = true; }

    /**
     * \brief Set the font for the the style
     * \param value QFont to be used for text
     */
    void setFont( const QFont &value ) { mFont = value; mValid = true; }

    /**
     * \brief Set the icon for the style. Icons are generated from symbols
     * \param value QgsSymbol to be used when generating the icon
     */
    void setSymbol( QgsSymbol *value );

    /**
     * \brief The name of the style.
     * \returns The name of the style. Names are optional so might be empty.
     */
    QString displayText() const;

    /**
     * \brief The name of the style.
     * \returns The name of the style. Names are optional so might be empty.
     */
    QString name() const { return mName; }

    /**
     * \brief The icon set for style generated from the set symbol
     * \returns A QPixmap that was set for the icon using the symbol
     */
    QPixmap icon() const { return mIcon; }

    /**
     * \brief The symbol used to generate the icon for the style
     * \returns The QgsSymbol used for the icon
     */
    QgsSymbol *symbol() const { return mSymbol.get(); }

    /**
     * \brief The text color set for style
     * \returns QColor for text color
     */
    QColor textColor() const { return mTextColor; }

    /**
     * \brief Check if the text color is valid for render.
     * Valid colors are non invalid QColors and a color with a > 0 alpha
     * \returns True of the color set for text is valid.
     */
    bool validTextColor() const;

    /**
     * \brief The background color for style
     * \returns QColor for background color
     */
    QColor backgroundColor() const { return mBackColor; }

    /**
     * \brief Check if the background color is valid for render.
     * Valid colors are non invalid QColors and a color with a > 0 alpha
     * \returns True of the color set for background is valid.
     */
    bool validBackgroundColor() const;

    /**
     * \brief The font for the style
     * \returns QFont for the style
     */
    QFont font() const { return mFont; }

    /**
     * \brief The condition rule set for the style. Rule may contain variable \@value
     * to represent the current value
     * \returns QString of the current set rule
     */
    QString rule() const { return mRule; }

    /**
     * \brief isValid Check if this rule is valid.  A valid rule has one or more properties
     * set.
     * \returns True if the rule is valid.
     */
    bool isValid() const { return mValid; }

    /**
     * \brief Find and return the matching styles for the value and feature.
     * If no match is found a invalid QgsConditionalStyle is return.
     *
     * \returns A conditional style that matches the value and feature.
     * Check with QgsConditionalStyle::isValid()
     */
    static QList<QgsConditionalStyle> matchingConditionalStyles( const QList<QgsConditionalStyle> &styles, const QVariant &value, QgsExpressionContext &context );

    /**
     * \brief Find and return the matching style for the value and feature.
     * If no match is found a invalid QgsConditionalStyle is return.
     *
     * \returns A conditional style that matches the value and feature.
     * Check with QgsConditionalStyle::isValid()
     */
    static QgsConditionalStyle matchingConditionalStyle( const QList<QgsConditionalStyle> &styles, const QVariant &value, QgsExpressionContext &context );

    /**
     * \brief Compress a list of styles into a single style.  This can be used to stack the elements of the
     * styles. The font of the last style is used in the output.
     * \param styles The list of styles to compress down
     * \returns A single style generated from joining each style property.
     */
    static QgsConditionalStyle compressStyles( const QList<QgsConditionalStyle> &styles );

    /** Reads vector conditional style specific state from layer Dom node.
     */
    bool readXml( const QDomNode &node, const QgsReadWriteContext &context );

    /** Write vector conditional style specific state from layer Dom node.
     */
    bool writeXml( QDomNode &node, QDomDocument &doc, const QgsReadWriteContext &context ) const;


  private:

    bool mValid = false;
    QString mName;
    QString mRule;
    std::unique_ptr<QgsSymbol> mSymbol;
    QFont mFont;
    QColor mBackColor;
    QColor mTextColor;
    QPixmap mIcon;
};

#endif // QGSCONDITIONALSTYLE_H
