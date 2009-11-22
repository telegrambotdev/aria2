/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2006 Tatsuhiro Tsujikawa
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * In addition, as a special exception, the copyright holders give
 * permission to link the code of portions of this program with the
 * OpenSSL library under certain conditions as described in each
 * individual source file, and distribute linked combinations
 * including the two.
 * You must obey the GNU General Public License in all respects
 * for all of the code used other than OpenSSL.  If you modify
 * file(s) with this exception, you may extend this exception to your
 * version of the file(s), but you are not obligated to do so.  If you
 * do not wish to do so, delete this exception statement from your
 * version.  If you delete this exception statement from all source
 * files in the program, then also delete it here.
 */
/* copyright --> */
#include "BtBitfieldMessage.h"

#include <cstring>

#include "bittorrent_helper.h"
#include "util.h"
#include "DlAbortEx.h"
#include "message.h"
#include "Peer.h"
#include "StringFormat.h"
#include "PieceStorage.h"
#include "a2functional.h"

namespace aria2 {

const std::string BtBitfieldMessage::NAME("bitfield");

void BtBitfieldMessage::setBitfield(const unsigned char* bitfield, size_t bitfieldLength) {
  if(this->bitfield == bitfield) {
    return;
  }
  delete [] this->bitfield;

  this->bitfieldLength = bitfieldLength;
  this->bitfield = new unsigned char[this->bitfieldLength];
  memcpy(this->bitfield, bitfield, this->bitfieldLength);
}

BtBitfieldMessageHandle
BtBitfieldMessage::create(const unsigned char* data, size_t dataLength)
{
  bittorrent::assertPayloadLengthGreater(1,dataLength, NAME);
  bittorrent::assertID(ID, data, NAME);
  BtBitfieldMessageHandle message(new BtBitfieldMessage());
  message->setBitfield(data+1, dataLength-1);
  return message;
}

void BtBitfieldMessage::doReceivedAction() {
  if(_metadataGetMode) {
    return;
  }
  pieceStorage->updatePieceStats(bitfield, bitfieldLength, peer->getBitfield());
  peer->setBitfield(bitfield, bitfieldLength);
  if(peer->isSeeder() && pieceStorage->downloadFinished()) {
    throw DL_ABORT_EX(MSG_GOOD_BYE_SEEDER);
  }
}

const unsigned char* BtBitfieldMessage::getMessage() {
  if(!msg) {
    /**
     * len --- 1+bitfieldLength, 4bytes
     * id --- 5, 1byte
     * bitfield --- bitfield, len bytes
     * total: 5+len bytes
     */
    msgLength = 5+bitfieldLength;
    msg = new unsigned char[msgLength];
    bittorrent::createPeerMessageString(msg, msgLength, 1+bitfieldLength, ID);
    memcpy(msg+5, bitfield, bitfieldLength);
  }
  return msg;
}

size_t BtBitfieldMessage::getMessageLength() {
  getMessage();
  return msgLength;
}

std::string BtBitfieldMessage::toString() const {
  return strconcat(NAME, " ", util::toHex(bitfield, bitfieldLength));
}

} // namespace aria2
