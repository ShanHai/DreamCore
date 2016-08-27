/*
 * DreamCore Auth DB base structure and records.
 *
 * Date: 2016-08-28 05:39:56
*/

-- ----------------------------
-- Table structure for dc_vip
-- ----------------------------
DROP TABLE IF EXISTS `dc_vip`;
CREATE TABLE `dc_vip` (
  `id` int(10) unsigned NOT NULL,
  `vipLevel` int(10) unsigned NOT NULL DEFAULT '0',
  `points` int(11) unsigned NOT NULL DEFAULT '0',
  `RealmID` int(11) NOT NULL DEFAULT '-1',
  PRIMARY KEY (`id`,`RealmID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

REPLACE INTO `rbac_linked_permissions` (`id`, `linkedId`) VALUES ('195', '25');
REPLACE INTO `rbac_linked_permissions` (`id`, `linkedId`) VALUES ('195', '26');
REPLACE INTO `rbac_linked_permissions` (`id`, `linkedId`) VALUES ('195', '29');

DELETE FROM `rbac_permissions` WHERE `id` BETWEEN 2000 AND 2017;

INSERT INTO `rbac_permissions` (`id`, `name`) VALUES ('2000', 'Command: dc');
INSERT INTO `rbac_permissions` (`id`, `name`) VALUES ('2001', 'Command: vip');
INSERT INTO `rbac_permissions` (`id`, `name`) VALUES ('2002', 'Command: dc reload');
INSERT INTO `rbac_permissions` (`id`, `name`) VALUES ('2003', 'Command: dc vip');
INSERT INTO `rbac_permissions` (`id`, `name`) VALUES ('2004', 'Command: dc reload system');
INSERT INTO `rbac_permissions` (`id`, `name`) VALUES ('2005', 'Command: dc reload strings');
INSERT INTO `rbac_permissions` (`id`, `name`) VALUES ('2006', 'Command: dc reload costs');
INSERT INTO `rbac_permissions` (`id`, `name`) VALUES ('2007', 'Command: dc reload requirements');
INSERT INTO `rbac_permissions` (`id`, `name`) VALUES ('2008', 'Command: dc reload icons');
INSERT INTO `rbac_permissions` (`id`, `name`) VALUES ('2009', 'Command: dc reload items');
INSERT INTO `rbac_permissions` (`id`, `name`) VALUES ('2010', 'Command: dc reload config');
INSERT INTO `rbac_permissions` (`id`, `name`) VALUES ('2011', 'Command: dc vip set');
INSERT INTO `rbac_permissions` (`id`, `name`) VALUES ('2012', 'Command: dc vip modify');
INSERT INTO `rbac_permissions` (`id`, `name`) VALUES ('2013', 'Command: dc vip set vipLevel');
INSERT INTO `rbac_permissions` (`id`, `name`) VALUES ('2014', 'Command: dc vip set points');
INSERT INTO `rbac_permissions` (`id`, `name`) VALUES ('2015', 'Command: dc vip modify points');
INSERT INTO `rbac_permissions` (`id`, `name`) VALUES ('2016', 'Command: dc vip add');
INSERT INTO `rbac_permissions` (`id`, `name`) VALUES ('2017', 'Command: dc vip add vendor');

DELETE FROM `rbac_linked_permissions` WHERE `linkedId` BETWEEN 2000 AND 2017;

INSERT INTO `rbac_linked_permissions` (`id`, `linkedId`) VALUES ('196', '2000');
INSERT INTO `rbac_linked_permissions` (`id`, `linkedId`) VALUES ('195', '2001');
INSERT INTO `rbac_linked_permissions` (`id`, `linkedId`) VALUES ('196', '2002');
INSERT INTO `rbac_linked_permissions` (`id`, `linkedId`) VALUES ('196', '2003');
INSERT INTO `rbac_linked_permissions` (`id`, `linkedId`) VALUES ('196', '2004');
INSERT INTO `rbac_linked_permissions` (`id`, `linkedId`) VALUES ('196', '2005');
INSERT INTO `rbac_linked_permissions` (`id`, `linkedId`) VALUES ('196', '2006');
INSERT INTO `rbac_linked_permissions` (`id`, `linkedId`) VALUES ('196', '2007');
INSERT INTO `rbac_linked_permissions` (`id`, `linkedId`) VALUES ('196', '2008');
INSERT INTO `rbac_linked_permissions` (`id`, `linkedId`) VALUES ('196', '2009');
INSERT INTO `rbac_linked_permissions` (`id`, `linkedId`) VALUES ('196', '2010');
INSERT INTO `rbac_linked_permissions` (`id`, `linkedId`) VALUES ('196', '2011');
INSERT INTO `rbac_linked_permissions` (`id`, `linkedId`) VALUES ('196', '2012');
INSERT INTO `rbac_linked_permissions` (`id`, `linkedId`) VALUES ('196', '2013');
INSERT INTO `rbac_linked_permissions` (`id`, `linkedId`) VALUES ('196', '2014');
INSERT INTO `rbac_linked_permissions` (`id`, `linkedId`) VALUES ('196', '2015');
INSERT INTO `rbac_linked_permissions` (`id`, `linkedId`) VALUES ('196', '2016');
INSERT INTO `rbac_linked_permissions` (`id`, `linkedId`) VALUES ('196', '2017');
