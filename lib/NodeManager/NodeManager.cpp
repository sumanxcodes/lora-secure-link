#include "NodeManager.h"

// Global container for tracking all peer states
std::vector<NodeState> peers;

/**
 * Find an existing peer by ID or create a new one.
 */
NodeState *findOrCreatePeer(const String &id)
{
    for (auto &peer : peers)
    {
        if (peer.id == id)
            return &peer;
    }

    NodeState newPeer;
    newPeer.id = id;
    newPeer.state = PeerState::IDLE;
    peers.push_back(newPeer);
    return &peers.back();
}

/**
 * Mark the peer as having received an ACK.
 * If both ACK and PK were exchanged, promote to SECURE_COMM.
 */
void markPeerAckReceived(const String &id)
{
    for (auto &peer : peers)
    {
        if (peer.id == id)
        {
            peer.ackReceived = true;

            if (peer.state == PeerState::ACK_PENDING && peer.ackSent)
            {
                peer.state = PeerState::SECURE_COMM;
                Serial.println("✅ ACK exchange complete. Transitioning to SECURE_COMM for " + peer.id);
            }

            break;
        }
    }
}

/**
 * Check if DH exchange is completed with this peer.
 */
bool isPeerDHComplete(const String &id)
{
    NodeState *peer = findOrCreatePeer(id);
    return peer->pkReceived && peer->ackReceived && peer->sharedSessionKey != 0;
}

/**
 * Reset a peer to its initial (IDLE) state.
 */
void resetPeer(NodeState *peer)
{
    if (!peer)
        return;

    peer->privateKey = 0;
    peer->publicKey = 0;
    peer->remotePublicKey = 0;
    peer->sharedSessionKey = 0;
    peer->pkSent = false;
    peer->pkReceived = false;
    peer->ackSent = false;
    peer->ackReceived = false;
    peer->messageCount = 0;
    peer->challenge = 0;
    peer->state = PeerState::IDLE;
}

/**
 * Print the current status of all connected peers.
 */
void printPeerStatus()
{
    Serial.println("\n========= Connected Nodes =========");
    for (const auto &peer : peers)
    {
        Serial.println("📡 ID: " + peer.id);
        Serial.println("🔑 Private Key: " + String(peer.privateKey));
        Serial.println("🔓 Public Key: " + String(peer.publicKey));
        Serial.println("🔒 Remote Public Key: " + String(peer.remotePublicKey));
        Serial.println("🤝 Shared Session Key: " + String(peer.sharedSessionKey));
        Serial.println("✅ PK Sent: " + String(peer.pkSent ? "Yes" : "No"));
        Serial.println("✅ PK Received: " + String(peer.pkReceived ? "Yes" : "No"));
        Serial.println("✅ ACK Received: " + String(peer.ackReceived ? "Yes" : "No"));
        Serial.println("-----------------------------------");
    }
    Serial.println("===================================\n");
}
