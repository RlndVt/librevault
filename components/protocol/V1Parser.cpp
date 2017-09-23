/* Copyright (C) 2015 Alexander Shishenko <alex@shishenko.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "V1Protocol.pb.h"
#include "V1Parser.h"

namespace librevault {

namespace {

template <class ProtoMessageClass>
QByteArray prepare_proto_message(const ProtoMessageClass& message_protobuf, V1Parser::message_type type) {
	QByteArray message_raw(1+message_protobuf.ByteSize(), 0);
	message_raw[0] = type;
	message_protobuf.SerializeToArray(message_raw.data()+1, message_protobuf.ByteSize());

	return message_raw;
}

}

QByteArray V1Parser::gen_Handshake(const Handshake& message_struct) {
	protocol::Handshake message_protobuf;
	message_protobuf.set_auth_token(message_struct.auth_token.toStdString());
	message_protobuf.set_device_name(message_struct.device_name.toStdString());
	message_protobuf.set_user_agent(message_struct.user_agent.toStdString());

	return prepare_proto_message(message_protobuf, HANDSHAKE);
}
V1Parser::Handshake V1Parser::parse_Handshake(QByteArray message_raw) {
	protocol::Handshake message_protobuf;
	if(!message_protobuf.ParseFromArray(message_raw.data()+1, message_raw.size()-1)) throw ParseError();

	Handshake message_struct;
	message_struct.auth_token = QByteArray::fromStdString(message_protobuf.auth_token());
	message_struct.device_name = QString::fromStdString(message_protobuf.device_name());
	message_struct.user_agent = QString::fromStdString(message_protobuf.user_agent());

	return message_struct;
}

QByteArray V1Parser::gen_HaveMeta(const HaveMeta& message_struct) {
	protocol::HaveMeta message_protobuf;

	message_protobuf.set_path_id(message_struct.revision.path_keyed_hash_.data(), message_struct.revision.path_keyed_hash_.size());
	message_protobuf.set_revision(message_struct.revision.revision_);

	QByteArray converted_bitfield = convert_bitfield(message_struct.bitfield);
	message_protobuf.set_bitfield(converted_bitfield.toStdString());

	return prepare_proto_message(message_protobuf, HAVE_META);
}
V1Parser::HaveMeta V1Parser::parse_HaveMeta(QByteArray message_raw) {
	protocol::HaveMeta message_protobuf;
	if(!message_protobuf.ParseFromArray(message_raw.data()+1, message_raw.size()-1)) throw ParseError();

	HaveMeta message_struct;
	message_struct.revision.revision_ = message_protobuf.revision();
	message_struct.revision.path_keyed_hash_ = QByteArray::fromStdString(message_protobuf.path_id());
	message_struct.bitfield = convert_bitfield(QByteArray::fromStdString(message_protobuf.bitfield()));

	return message_struct;
}

QByteArray V1Parser::gen_HaveChunk(const HaveChunk& message_struct) {
	protocol::HaveChunk message_protobuf;

	message_protobuf.set_ct_hash(message_struct.ct_hash.data(), message_struct.ct_hash.size());

	return prepare_proto_message(message_protobuf, HAVE_CHUNK);
}
V1Parser::HaveChunk V1Parser::parse_HaveChunk(QByteArray message_raw) {
	protocol::HaveChunk message_protobuf;
	if(!message_protobuf.ParseFromArray(message_raw.data()+1, message_raw.size()-1)) throw ParseError();

	HaveChunk message_struct;
	message_struct.ct_hash = QByteArray::fromStdString(message_protobuf.ct_hash());

	return message_struct;
}

QByteArray V1Parser::gen_MetaRequest(const MetaRequest& message_struct) {
	protocol::MetaRequest message_protobuf;
	message_protobuf.set_path_id(message_struct.revision.path_keyed_hash_.data(), message_struct.revision.path_keyed_hash_.size());
	message_protobuf.set_revision(message_struct.revision.revision_);

	return prepare_proto_message(message_protobuf, META_REQUEST);
}
V1Parser::MetaRequest V1Parser::parse_MetaRequest(QByteArray message_raw) {
	protocol::MetaRequest message_protobuf;
	if(!message_protobuf.ParseFromArray(message_raw.data()+1, message_raw.size()-1)) throw ParseError();

	MetaRequest message_struct;
	message_struct.revision.path_keyed_hash_ = QByteArray::fromStdString(message_protobuf.path_id());
	message_struct.revision.revision_ = message_protobuf.revision();

	return message_struct;
}

QByteArray V1Parser::gen_MetaReply(const MetaReply& message_struct) {
	protocol::MetaReply message_protobuf;
	message_protobuf.set_meta(message_struct.smeta.rawMetaInfo().data(), message_struct.smeta.rawMetaInfo().size());
	message_protobuf.set_signature(message_struct.smeta.signature().data(), message_struct.smeta.signature().size());
	message_protobuf.set_bitfield(convert_bitfield(message_struct.bitfield).toStdString());

	return prepare_proto_message(message_protobuf, META_REPLY);
}
V1Parser::MetaReply V1Parser::parse_MetaReply(QByteArray message_raw) {
	protocol::MetaReply message_protobuf;
	if(!message_protobuf.ParseFromArray(message_raw.data()+1, message_raw.size()-1)) throw ParseError();

	QByteArray raw_meta = QByteArray::fromStdString(message_protobuf.meta());
	QByteArray signature = QByteArray::fromStdString(message_protobuf.signature());

	QBitArray converted_bitfield = convert_bitfield(QByteArray::fromStdString(message_protobuf.bitfield()));

	return MetaReply{SignedMeta(std::move(raw_meta), std::move(signature)), std::move(converted_bitfield)};
}

QByteArray V1Parser::gen_BlockRequest(const BlockRequest& message_struct) {
	protocol::BlockRequest message_protobuf;
	message_protobuf.set_ct_hash(message_struct.ct_hash.data(), message_struct.ct_hash.size());
	message_protobuf.set_offset(message_struct.offset);
	message_protobuf.set_length(message_struct.length);

	return prepare_proto_message(message_protobuf, BLOCK_REQUEST);
}
V1Parser::BlockRequest V1Parser::parse_BlockRequest(QByteArray message_raw) {
	protocol::BlockRequest message_protobuf;
	if(!message_protobuf.ParseFromArray(message_raw.data()+1, message_raw.size()-1)) throw ParseError();

	BlockRequest message_struct;
	message_struct.ct_hash = QByteArray::fromStdString(message_protobuf.ct_hash());
	message_struct.offset = message_protobuf.offset();
	message_struct.length = message_protobuf.length();

	return message_struct;
}

QByteArray V1Parser::gen_BlockReply(const BlockReply& message_struct) {
	protocol::BlockReply message_protobuf;
	message_protobuf.set_ct_hash(message_struct.ct_hash.data(), message_struct.ct_hash.size());
	message_protobuf.set_offset(message_struct.offset);
	message_protobuf.set_content(message_struct.content.data(), message_struct.content.size());

	return prepare_proto_message(message_protobuf, BLOCK_REPLY);
}
V1Parser::BlockReply V1Parser::parse_BlockReply(QByteArray message_raw) {
	protocol::BlockReply message_protobuf;
	if(!message_protobuf.ParseFromArray(message_raw.data()+1, message_raw.size()-1)) throw ParseError();

	BlockReply message_struct;
	message_struct.ct_hash = QByteArray::fromStdString(message_protobuf.ct_hash());
	message_struct.offset = message_protobuf.offset();
	message_struct.content = QByteArray::fromStdString(message_protobuf.content());

	return message_struct;
}

} /* namespace librevault */
