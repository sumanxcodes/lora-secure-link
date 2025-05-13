#ifndef NODE_MANAGER_H
#define NODE_MANAGER_H

#include <Arduino.h>
#include <vector>

// ========== ENUM: Peer FSM States ==========
enum class PeerState
{
    IDLE,         // Default/reset state
    ACK_PENDING,  // Waiting for ACK exchange
    SECURE_COMM,  // DH key exchange complete
    CHAL_SENT,    // CHAL sent, waiting for RESP
    AUTHENTICATED // Fully authenticated, secure channel
};

// ========== STRUCT: NodeState ==========
// Represents the status and keys for each peer
struct NodeState
{
    String id;

    // Key material
    uint32_t privateKey = 0;
    uint32_t publicKey = 0;
    uint32_t remotePublicKey = 0;
    uint32_t sharedSessionKey = 0;

    // Handshake tracking
    bool pkSent = false;
    bool pkReceived = false;
    bool ackSent = false;
    bool ackReceived = false;

    // Challenge-response
    uint32_t challenge = 0;
    uint32_t messageCount = 0;

    PeerState state = PeerState::IDLE;
};

// ========== Peer Management ==========
extern std::vector<NodeState> peers;

NodeState *findOrCreatePeer(const String &id);
void markPeerAckReceived(const String &id);
bool isPeerDHComplete(const String &id);
void resetPeer(NodeState *peer);
void printPeerStatus();

#endif
