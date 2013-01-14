/*
 * overlay.h
 *
 *  Created on: 8 déc. 2012
 *      Author: Utilisateur
 */

#ifndef OVERLAY_H_
#define OVERLAY_H_

#include <QtGui/QWidget>

class Overlay {
public:
	Overlay() {
	}
	virtual ~Overlay() {
	}
	virtual void expose() = 0;
};

#endif /* OVERLAY_H_ */
